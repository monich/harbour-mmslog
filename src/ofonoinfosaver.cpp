/*
  Copyright (C) 2015 Jolla Ltd.
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

#include "ofonoinfosaver.h"
#include "manager_interface.h"

#include <fcntl.h>
#include <unistd.h>

#define OFONO_SERVICE "org.ofono"
#define OFONO_INTERFACE "org.ofono"

OfonoInfoSaver::OfonoInfoSaver(QString aDir, QObject* aParent) :
    QObject(aParent),
    iDir(aDir),
    iOfonoManager(new OrgOfonoManager(OFONO_SERVICE, "/",
        QDBusConnection::systemBus(), this))
{
    connect(new QDBusPendingCallWatcher(iOfonoManager->GetModems(), this),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetModemsFinished(QDBusPendingCallWatcher*)));
}

void OfonoInfoSaver::onGetModemsFinished(QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<OfonoObjectPathPropertiesList> reply(*aCall);
    aCall->deleteLater();
    if (reply.isError()) {
        qWarning() << reply.error().message();
    } else {
        OfonoObjectPathPropertiesList list = reply.value();
        const int n = list.count();
        for (int i=0; i<n; i++) {
            QString modem(list[i].path.path());
            saveInfo(modem, "SimManager.GetProperties");
            saveInfo(modem, "ConnectionManager.GetContexts");
            saveInfo(modem, "NetworkRegistration.GetProperties");
        }
    }
}

void OfonoInfoSaver::saveInfo(QString aModemPath, const char* aCall)
{
    QString call(OFONO_INTERFACE);
    call += ".";
    call += aCall;

    QString file(iDir);
    file += "/";
    file += aCall;
    file += QString(aModemPath).replace('/', '.');
    file += ".txt";

    int fd = open(qPrintable(file), O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        if (fork() == 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            execlp("dbus-send", "dbus-send", "--system", "--print-reply",
                "--dest=" OFONO_SERVICE, qPrintable(aModemPath),
                qPrintable(call), NULL);
        }
        close(fd);
    }
}

void OfonoInfoSaver::registerType()
{
    qDBusRegisterMetaType<OfonoObjectPathProperties>();
    qDBusRegisterMetaType<OfonoObjectPathPropertiesList>();
}

QDBusArgument& operator<<(QDBusArgument& aArg, const OfonoObjectPathProperties& aList)
{
    aArg.beginStructure();
    aArg << aList.path << aList.properties;
    aArg.endStructure();
    return aArg;
}

const QDBusArgument& operator>>(const QDBusArgument& aArg, OfonoObjectPathProperties& aList)
{
    aArg.beginStructure();
    aArg >> aList.path >> aList.properties;
    aArg.endStructure();
    return aArg;
}
