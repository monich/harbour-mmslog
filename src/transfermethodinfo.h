/*
  Copyright (C) 2014 Jolla Ltd.
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

#ifndef TRANSFERMETHODINFO_H
#define TRANSFERMETHODINFO_H

#include <QtDBus/QtDBus>

class TransferMethodInfo
{
public:
    TransferMethodInfo() {}
    TransferMethodInfo &operator=(const TransferMethodInfo& that);
    TransferMethodInfo(const TransferMethodInfo& that);
    bool operator==(const TransferMethodInfo& that) const;
    bool operator!=(const TransferMethodInfo& that) const;
    bool equals(const TransferMethodInfo& that) const;

    static void registerType();

    QString displayName;
    QString userName;
    QString methodId;
    QString shareUIPath;
    QStringList capabilitities;
    quint32 accountId;
};

typedef QList<TransferMethodInfo> TransferMethodInfoList;
QDBusArgument& operator<<(QDBusArgument& aArg, const TransferMethodInfo& aInfo);
const QDBusArgument& operator>>(const QDBusArgument& aArg, TransferMethodInfo& aInfo);

Q_DECLARE_METATYPE(TransferMethodInfo)

#endif // TRANSFERMETHODINFO_H
