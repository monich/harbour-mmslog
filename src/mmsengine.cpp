/*
  Copyright (C) 2014-2015 Jolla Ltd.
  Contact: Slava Monich <slava.monich@jolla.com>
  All rights reserved.

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

#include "mmsengine.h"
#include "mmsenginelog.h"
#include "mmsdebug.h"

#include <QDir>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>

#define MMS_ENGINE "/usr/sbin/mms-engine"

MMSEngine::MMSEngine(QString aTempDir, QObject* aParent) :
    QObject(aParent),
    iTempDir(aTempDir),
    iPid(-1),
    iPipe(-1)
{
    killEngine();
    iEngineLog = startEngine();
}

MMSEngine::~MMSEngine()
{
    LOG("terminating");
    if (iPid > 0) kill(iPid, SIGTERM);
    if (iPipe >= 0) close(iPipe);
    if (iEngineLog) {
        iEngineLog->wait();
        delete iEngineLog;
    }
}

void MMSEngine::killEngine()
{
    QDir proc("/proc");
    QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (QString entry, entries) {
        int pid = entry.toInt();
        if (pid) {
            QString exe = QFile::symLinkTarget(proc.filePath(entry).append("/exe"));
            if (!exe.isEmpty() && QFile::exists(exe) && exe == MMS_ENGINE) {
                LOG("MMS engine already running, pid" << pid);
                kill(pid, SIGTERM);
                break;
            }
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
            LOG("pty =" << ptyname);
            iPid = fork();
            if (iPid > 0) {
                // Parent
                LOG("Stared MMS engine, pid" << iPid);
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
        LOG("mms-engine died, pid" << aPid <<", status" << aStatus);
        iPid = -1;
        if (iPipe < 0) {
            engineDied();
        }
    }
}

void MMSEngine::pipeClosed(int aPipe)
{
    LOG("pipe" << aPipe << "closed");
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
    emit message("MMS engine died", false);
}

void MMSEngine::forward(QString aMessage)
{
    LOG(aMessage);
    emit message(aMessage, true);
}

MMSEngineLog* MMSEngine::startLogThread(int aDescriptor)
{
    MMSEngineLog* logThread = new MMSEngineLog(aDescriptor);
    connect(logThread, SIGNAL(finished()), logThread, SLOT(deleteLater()));
    connect(logThread, SIGNAL(message(QString)),
        SLOT(forward(QString)), Qt::QueuedConnection);
    connect(logThread, SIGNAL(done(int)),
        SLOT(pipeClosed(int)), Qt::QueuedConnection);
    logThread->start();
    return logThread;
}
