/*
  Copyright (C) 2014-2015 Jolla Ltd.
  Contact: Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Jolla Ltd nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

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

#include "mmslogmodel.h"
#include "mmsengine.h"
#include "mmsdebug.h"
#include "sigchildaction.h"
#include "transfermethodsmodel.h"

#include <sailfishapp.h>
#include <QGuiApplication>
#include <QtQuick>
#include <QtQml>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define PLUGIN_PREFIX "harbour.mmslog"

static SigChildAction* sigChildHandler = NULL;

static
void
sigchild_action(
    int,
    siginfo_t* aInfo,
    void*)
{
    int status = -1;
    waitpid(aInfo->si_pid, &status, 0);
    if (sigChildHandler) {
        sigChildHandler->notify(aInfo->si_pid, status);
    }
}

static
void
save_ofono_info(
    const char* aCall,
    const char* aFile)
{
    int fd = open(aFile, O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        if (fork() == 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            execlp("dbus-send", "dbus-send", "--system", "--print-reply",
                "--dest=org.ofono", "/ril_0", aCall, NULL);
        }
        close(fd);
    }
}

static void register_types(const char* uri, int v1 = 1, int v2 = 0)
{
    qmlRegisterType<FsIoLogModel>(uri, v1, v2, "FsIoLogModel");
    qmlRegisterType<TransferMethodsModel>(uri, v1, v2, "TransferMethodsModel");
}

static bool load_transferengine_translations(QTranslator* tr, QLocale locale)
{
    if (tr->load(locale, "sailfish_transferengine_plugins", "-",
        "/usr/share/translations")) {
        return true;
    } else {
        LOG("Failed to load transferengine plugin translator for" << locale);
        return false;
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    TransferMethodInfo::registerType();
    register_types(PLUGIN_PREFIX, 1, 0);

    // Load translations
    QLocale locale;
    QTranslator* translator = new QTranslator(app);
    QString transDir = SailfishApp::pathTo("translations").toLocalFile();
    QString transFile("harbour-mmslog");
    if (translator->load(locale, transFile, "-", transDir) ||
        translator->load(transFile, transDir)) {
        app->installTranslator(translator);
    } else {
        LOG("Failed to load translator for" << locale);
        delete translator;
    }
    translator = new QTranslator(app);
    if (load_transferengine_translations(translator, locale) ||
        load_transferengine_translations(translator, QLocale("en_GB"))) {
        app->installTranslator(translator);
    } else {
        delete translator;
    }

    // Install signal handler
    sigChildHandler = SigChildAction::create(app);
    if (sigChildHandler) {
        struct sigaction act;
        memset (&act, '\0', sizeof(act));
        act.sa_sigaction = &sigchild_action;
        act.sa_flags = SA_SIGINFO;
        sigaction(SIGCHLD, &act, NULL);
    }

    // Start (or restart) mms-engine
    FsIoLogModel* mmsLog = new FsIoLogModel(app);
    MMSEngine* mmsEngine = new MMSEngine(mmsLog->dirName(), app);
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
    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->showFullScreen();

    save_ofono_info("org.ofono.SimManager.GetProperties",
        qPrintable(mmsLog->dirName() + "/SimManager.GetProperties.txt"));
    save_ofono_info("org.ofono.ConnectionManager.GetContexts",
        qPrintable(mmsLog->dirName() + "/ConnectionManager.GetContexts.txt"));
    save_ofono_info("org.ofono.NetworkRegistration.GetProperties",
        qPrintable(mmsLog->dirName() + "/NetworkRegistration.GetProperties.txt"));

    int ret = app->exec();

    sigChildHandler = NULL;
    delete view;
    delete app;
    return ret;
}
