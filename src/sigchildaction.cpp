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

#include "sigchildaction.h"
#include "mmsdebug.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

SigChildAction::SigChildAction(int aFd[], QObject* aParent) :
    QSocketNotifier(aFd[1], QSocketNotifier::Read, aParent)
{
    iFd[0] = aFd[0];
    iFd[1] = aFd[1];
    connect(this, SIGNAL(activated(int)), SLOT(handleSigChild()));
}

SigChildAction::~SigChildAction()
{
    close(iFd[0]);
    close(iFd[1]);
}

SigChildAction* SigChildAction::create(QObject* aParent)
{
    int fd[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == 0) {
        return new SigChildAction(fd, aParent);
    } else {
        qWarning() << "Failed to create socket pair";
        return NULL;
    }
}

bool SigChildAction::notify(int aPid, int aStatus)
{
    int data[2];
    data[0] = aPid;
    data[1] = aStatus;
    return (write(iFd[0], data, sizeof(data)) == sizeof(data));
}

void SigChildAction::handleSigChild()
{
    int data[2];
    if (read(iFd[1], data, sizeof(data)) == sizeof(data)) {
        LOG("Child" << data[0] << "died, status" << data[1]);
        Q_EMIT processDied(data[0], data[1]);
    } else {
        qWarning() << "Error handling SIGCHLD";
    }
}
