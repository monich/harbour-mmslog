/*
  Copyright (C) 2014-2020 Jolla Ltd.
  Copyright (C) 2014-2020 Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the names of the copyright holders nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "appsettings.h"
#include "mmslogmodel.h"
#include "mmsengine.h"
#include "ofonologger.h"

#include "HarbourDebug.h"
#include "HarbourSigChildHandler.h"
#include "HarbourTransferMethodsModel.h"

#include <sailfishapp.h>
#include <QGuiApplication>
#include <QtQuick>
#include <QtQml>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/fsuid.h>

#define PLUGIN_PREFIX "harbour.mmslog"

static void save_disk_usage(QString dir)
{
    QString file(dir);
    file += "/storage";
    int fd = open(qPrintable(file), O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        if (fork() == 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            execlp("df", "-k", "/tmp", qPrintable(QDir::homePath()), NULL);
        }
        close(fd);
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    HarbourTransferMethodInfo2::registerTypes();
    OfonoLogger::registerType();

    // Load translations
    QLocale locale;
    QTranslator* ts = new QTranslator(app);
#ifdef OPENREPOS
    // OpenRepos build has settings applet
    const bool appSettingsMenu = false;
    const QString transDir("/usr/share/translations");
    const QString transFile("openrepos-mmslog");
#else
    const bool appSettingsMenu = true;
    const QString transDir = SailfishApp::pathTo("translations").toLocalFile();
    const QString transFile("harbour-mmslog");
#endif
    if (ts->load(locale, transFile, "-", transDir) ||
        ts->load(transFile, transDir)) {
        app->installTranslator(ts);
    } else {
        HDEBUG("Failed to load translator for" << locale);
        delete ts;
    }
    ts = new QTranslator(app);
    if (HarbourTransferMethodsModel::loadTranslations(ts, locale) ||
        HarbourTransferMethodsModel::loadTranslations(ts, QLocale("en"))) {
        app->installTranslator(ts);
    } else {
        delete ts;
    }

    // Install signal handler
    HarbourSigChildHandler* sigChildHandler =
        HarbourSigChildHandler::install(app);

    // Start (or restart) mms-engine
    AppSettings* settings = new AppSettings(app);
    FsIoLogModel* mmsLog = new FsIoLogModel(settings, app);
    MMSEngine* mmsEngine = new MMSEngine(mmsLog->dirName(), app);
    HarbourTransferMethodsModel* tm = new HarbourTransferMethodsModel(app);
    tm->setFilter(mmsLog->archiveType());
    mmsLog->connect(mmsEngine, SIGNAL(message(QString,bool)),
        SLOT(append(QString,bool)));
    if (sigChildHandler) {
        mmsEngine->connect(sigChildHandler,
            SIGNAL(processDied(int,int)),
            SLOT(processDied(int,int)));
        mmsLog->connect(sigChildHandler,
            SIGNAL(processDied(int,int)),
            SLOT(processDied(int,int)));
    }

    // Create ans show the view
    QQuickView* view = SailfishApp::createView();
    QQmlContext* context = view->rootContext();
    context->setContextProperty("FsIoLog", mmsLog);
    context->setContextProperty("TransferMethodsModel", tm);
    context->setContextProperty("AppSettings", settings);
    context->setContextProperty("AppSettings", settings);
    context->setContextProperty("AppSettingsMenu",
        QVariant::fromValue(appSettingsMenu));

    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->showFullScreen();

    QString dir(mmsLog->dirName());
    save_disk_usage(dir);
    QFile::copy("/etc/sailfish-release", dir + "/sailfish-release");
    OfonoLogger* ofono = new OfonoLogger(dir, settings, app);
    ofono->connect(mmsLog, SIGNAL(flushed()), SLOT(flush()));
    int ret = app->exec();

    sigChildHandler = NULL;
    delete view;
    delete app;
    return ret;
}
