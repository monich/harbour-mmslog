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

#include "transfermethodsmodel.h"
#include "org.nemo.transferengine.h"
#include "mmsdebug.h"

TransferMethodsModel::TransferMethodsModel(QObject* aParent):
    QAbstractListModel(aParent)
{
    iTransferEngine = new OrgNemoTransferEngine("org.nemo.transferengine",
        "/org/nemo/transferengine", QDBusConnection::sessionBus(), this);
    connect(iTransferEngine,
        SIGNAL(transferMethodListChanged()),
        SLOT(requestUpdate()));
    requestUpdate();
}

TransferMethodsModel::~TransferMethodsModel()
{
    delete iTransferEngine;
}

void TransferMethodsModel::requestUpdate()
{
    connect(new QDBusPendingCallWatcher(
        iTransferEngine->transferMethods(), this),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onTransferMethodsFinished(QDBusPendingCallWatcher*)));
}

void TransferMethodsModel::onTransferMethodsFinished(QDBusPendingCallWatcher* aWatch)
{
    QDBusPendingReply<TransferMethodInfoList> reply(*aWatch);
    aWatch->deleteLater();
    if (reply.isError()) {
        qWarning() << reply.error().message();
    } else {
        TransferMethodInfoList list = reply.value();
        LOG(list.count() << "methods");
        if (iMethodList != list) {
            iMethodList = list;
            filterModel();
        }
    }
}

QRegExp TransferMethodsModel::regExp(QString aRegExp)
{
    return QRegExp(aRegExp, Qt::CaseInsensitive, QRegExp::Wildcard);
}

void TransferMethodsModel::filterModel()
{
    beginResetModel();
    const int oldCount = iFilteredList.count();
    iFilteredList.clear();
    if (iFilter.isEmpty() || iFilter == "*") {
        LOG("no filter");
        for (int i=0; i<iMethodList.count(); i++) {
            iFilteredList.append(i);
        }
    } else {
        QRegExp re(regExp(iFilter));
        for (int i=0; i<iMethodList.count(); i++) {
            const TransferMethodInfo& info = iMethodList.at(i);
            for (int j=0; j<info.capabilitities.count(); j++) {
                const QString& cap = info.capabilitities.at(j);
                if (iFilter == cap ||
                    re.exactMatch(cap) ||
                    regExp(cap).exactMatch(iFilter)) {
                    LOG(i << ":" << iFilter << "matches" << cap);
                    iFilteredList.append(i);
                    break;
                } else {
                    LOG(i << ":" << iFilter << "doesn't match" << cap);
                }
            }
        }
    }
    if (oldCount != iFilteredList.count()) {
        Q_EMIT countChanged();
    }
    endResetModel();
}

QHash<int,QByteArray> TransferMethodsModel::roleNames() const
{
    QHash<int,QByteArray> roles;
    roles[DisplayNameRole] = "displayName";
    roles[UserNameRole]    = "userName";
    roles[MethodIdRole]    = "methodId";
    roles[ShareUIPathRole] = "shareUIPath";
    roles[AccountIdRole]   = "accountId";
    return roles;
}

int TransferMethodsModel::rowCount(const QModelIndex &) const
{
    return iFilteredList.count();
}

QVariant TransferMethodsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= 0 && row < iFilteredList.count()) {
        const TransferMethodInfo& info = iMethodList.at(iFilteredList.at(row));
        switch (role) {
        case DisplayNameRole: return info.displayName;
        case UserNameRole:    return info.userName;
        case MethodIdRole:    return info.methodId;
        case ShareUIPathRole: return info.shareUIPath;
        case AccountIdRole:   return info.accountId;
        }
    }
    qWarning() << index << role;
    return QVariant();
}

int TransferMethodsModel::count() const
{
    return iFilteredList.count();
}

QString TransferMethodsModel::filter() const
{
    return iFilter;
}

void TransferMethodsModel::setFilter(QString aFilter)
{
    if (iFilter != aFilter) {
        iFilter = aFilter;
        filterModel();
        Q_EMIT filterChanged();
    }
}
