/*
 * Copyright (C) 2014-2021 Jolla Ltd.
 * Copyright (C) 2014-2021 Slava Monich <slava.monich@jolla.com>
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mmsengine.h"
#include "mmsenginelog.h"

#include "dbusaccess_peer.h"

#include "HarbourDebug.h"

#include <QDir>
#include <QDBusMessage>
#include <QDBusConnection>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>

#define MMS_ENGINE_BUS QDBusConnection::systemBus()
#define MMS_ENGINE "/usr/sbin/mms-engine"
#define MMS_ENGINE_NAME "org.nemomobile.MmsEngine"
#define MMS_ENGINE_IFACE "org.nemomobile.MmsEngine"
#define MMS_ENGINE_RESTART_DELAY (1000)
#define MMS_ENGINE_MAX_RESTARTS  (10)

MMSEngine::MMSEngine(QString aTempDir, QObject* aParent) :
    QObject(aParent),
    iEngineLog(NULL),
    iRestartTimer(NULL),
    iRestartCount(0),
    iTempDir(aTempDir),
    iPid(-1),
    iPipe(-1)
{
    killEngine();
    iEngineLog = startEngine();
}

MMSEngine::~MMSEngine()
{
    HDEBUG("terminating");
    stopLogging();
}

void MMSEngine::stopLogging()
{
    if (iPid > 0) {
        kill(iPid, SIGTERM);
        iPid = -1;
    }
    if (iPipe >= 0) {
        close(iPipe);
        iPipe = -1;
    }
    if (iEngineLog) {
        iEngineLog->wait();
        delete iEngineLog;
        iEngineLog = NULL;
    }
}

void MMSEngine::killEngine()
{
    DAPeer* peer = da_peer_get(DA_BUS_SYSTEM, MMS_ENGINE_NAME);
    if (peer) {
        HDEBUG("MMS engine already running, pid" << peer->pid);

        /* Try to stop it with a D-Bus request (works wince mms-engine 1.0.70) */
        if (MMS_ENGINE_BUS.call(QDBusMessage::createMethodCall(MMS_ENGINE_NAME, "/",
            MMS_ENGINE_IFACE, "exit")).type() == QDBusMessage::ReplyMessage) {
            HDEBUG("MMS engine stopped over D-Bus");
        } else if (kill(peer->pid, SIGTERM) < 0) {
            HWARN("Failed to kill MMS engine:" << strerror(errno));
        }
    }
}

MMSEngineLog* MMSEngine::startEngine()
{
    // Use a pseudoterminal to get around the block buffering performed
    // by the stdio library when standard output is redirected to a file
    // or pipe. We need line-buffering to watch the output in real time.
    const char* ptyname = NULL;
    pid_t pty = posix_openpt(O_RDWR | O_NOCTTY);
    if (pty >= 0) {
        if (!grantpt(pty) &&
            !unlockpt(pty) &&
            (ptyname = ptsname(pty)) != NULL) {
            HDEBUG("pty =" << ptyname);
            iPid = fork();
            if (iPid > 0) {
                // Parent
                HDEBUG("Started MMS engine, pid" << iPid);
                return startLogThread(iPipe = pty);
            } else if (iPid == 0) {
                // Child
                close(pty);
                prctl(PR_SET_PDEATHSIG, SIGTERM);
                pty = open(ptyname, O_WRONLY);
                dup2(pty, STDOUT_FILENO);
                dup2(pty, STDERR_FILENO);
                close(pty);
                execlp(MMS_ENGINE, MMS_ENGINE, "-vkat", "-d",
                    qPrintable(iTempDir), NULL);
                fprintf(stderr, "%s", strerror(errno));
                fflush(stderr);
                exit(1);
            } else {
                qWarning() << "Failed to launch mms-engine";
            }
        }
        close(pty);
    }
    return NULL;
}

void MMSEngine::processDied(int aPid, int aStatus)
{
    if (iPid > 0 && iPid == aPid) {
        HDEBUG("mms-engine died, pid" << aPid <<", status" << aStatus);
        iPid = -1;
        if (iPipe < 0) {
            engineDied();
        }
    }
}

void MMSEngine::pipeClosed(int aPipe)
{
    HDEBUG("pipe" << aPipe << "closed");
    if (iPipe >= 0 && iPipe == aPipe) {
        close(iPipe);
        iPipe = -1;
        if (iPid < 0) {
            engineDied();
        }
    }
}

void MMSEngine::engineDied()
{
    emit warning("MMS engine died");
    if (iRestartCount < MMS_ENGINE_MAX_RESTARTS) {
        if (!iRestartTimer) {
            iRestartTimer = new QTimer(this);
            iRestartTimer->setSingleShot(true);
            iRestartTimer->setInterval(MMS_ENGINE_RESTART_DELAY);
            connect(iRestartTimer, SIGNAL(timeout()), SLOT(restart()));
        }
        iRestartTimer->start();
        iRestartCount++;
    }
}

void MMSEngine::restart()
{
    if (iRestartTimer) {
        delete iRestartTimer;
        iRestartTimer = NULL;
    }
    stopLogging();
    iEngineLog = startEngine();
}

MMSEngineLog* MMSEngine::startLogThread(int aDescriptor)
{
    MMSEngineLog* logThread = new MMSEngineLog(aDescriptor);
    connect(logThread, SIGNAL(message(QString)),
        SIGNAL(logMessage(QString)), Qt::QueuedConnection);
    connect(logThread, SIGNAL(done(int)),
        SLOT(pipeClosed(int)), Qt::QueuedConnection);
    logThread->start();
    return logThread;
}
