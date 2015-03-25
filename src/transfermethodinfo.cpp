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

#include "transfermethodinfo.h"

QDBusArgument &operator<<(QDBusArgument &arg, const TransferMethodInfo &info)
{
    arg.beginStructure();
    arg << info.displayName << info.userName << info.methodId
        << info.shareUIPath << info.capabilitities << info.accountId;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, TransferMethodInfo &info)
{
    arg.beginStructure();
    arg >> info.displayName >> info.userName >> info.methodId
        >> info.shareUIPath >> info.capabilitities >> info.accountId;
    arg.endStructure();
    return arg;
}

TransferMethodInfo::TransferMethodInfo(const TransferMethodInfo &that):
    displayName(that.displayName),
    userName(that.userName),
    methodId(that.methodId),
    shareUIPath(that.shareUIPath),
    capabilitities(that.capabilitities),
    accountId(that.accountId)
{
}

TransferMethodInfo& TransferMethodInfo::operator=(const TransferMethodInfo& that)
{
    this->displayName    = that.displayName;
    this->userName       = that.userName;
    this->methodId       = that.methodId;
    this->shareUIPath    = that.shareUIPath;
    this->capabilitities = that.capabilitities;
    this->accountId      = that.accountId;
    return *this;
}

bool TransferMethodInfo::equals(const TransferMethodInfo& that) const
{
    return this->displayName    == that.displayName    &&
           this->userName       == that.userName       &&
           this->methodId       == that.methodId       &&
           this->shareUIPath    == that.shareUIPath    &&
           this->capabilitities == that.capabilitities &&
           this->accountId      == that.accountId;
}

bool TransferMethodInfo::operator==(const TransferMethodInfo& that) const
{
    return equals(that);
}

bool TransferMethodInfo::operator!=(const TransferMethodInfo& that) const
{
    return !equals(that);
}

void TransferMethodInfo::registerType()
{
    qDBusRegisterMetaType<TransferMethodInfo>();
    qDBusRegisterMetaType<TransferMethodInfoList>();
}
