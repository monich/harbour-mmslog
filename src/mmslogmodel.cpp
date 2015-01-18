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

#include "mmslogmodel.h"
#include "mmsdebug.h"

#include <QDateTime>

#include <signal.h>

#define LOG_FILE                "mms-engine.log"
#define DCONF_PATH              "/apps/harbour-mmslog/"
#define DCONF_LOG_SIZE_LIMIT    "logSizeLimit"

#define LOG_SIZE_LIMIT_DEFAULT  (1000)
#define LOG_SIZE_LIMIT_MIN      (100)
#define LOG_SIZE_LIMIT_NONE     (0)
#define LOG_REMOVE_MAX          (20)
#define LOG_REMOVE_DEFAULT      LOG_REMOVE_MAX

enum FsIoLogRole {
    TimeRole = Qt::UserRole,
    TextRole
};

class FsIoLogModel::Entry {
public:
    Entry(QString aMessage, bool aFromMmsEngine);
    QString iMessage;
    QString iTimestamp;
    QString iText;
};

FsIoLogModel::Entry::Entry(QString aMessage, bool aFromMmsEngine) :
    iMessage(aMessage)
{
    // MMS engine starts each line with YYYY-MM-DD hh:mm:ss
    // Skip the date part and split time and text
    if (aFromMmsEngine &&
        aMessage.length() > 20 &&   // YYYY-MM-DD
        aMessage[0].isDigit() && aMessage[1].isDigit() &&
        aMessage[2].isDigit() && aMessage[3].isDigit() &&
        aMessage[4] == '-' &&
        aMessage[5].isDigit() && aMessage[6].isDigit() &&
        aMessage[7] == '-' &&
        aMessage[8].isDigit() && aMessage[9].isDigit() &&
        aMessage[10] == ' ' &&      // hh:mm:ss
        aMessage[11].isDigit() && aMessage[12].isDigit() &&
        aMessage[13] == ':' &&
        aMessage[14].isDigit() && aMessage[15].isDigit() &&
        aMessage[16] == ':' &&
        aMessage[17].isDigit() && aMessage[18].isDigit() &&
        aMessage[19] == ' ') {
        iText = aMessage.right(aMessage.length()-20);
        iTimestamp = aMessage.mid(11, 8);
    } else {
        iText = aMessage;
    }
}

FsIoLogModel::FsIoLogModel(QObject* aParent) :
    QAbstractListModel(aParent),
    iTempDir("/tmp/mms_XXXXXX"),
    iLogFile(iTempDir.path().append("/" LOG_FILE)),
    iArchiveType("application/x-gzip"),
    iLogSizeLimitConf(new MGConfItem(DCONF_PATH DCONF_LOG_SIZE_LIMIT, this)),
    iPid(-1)
{
    updateLogSizeLimit();
    connect(iLogSizeLimitConf,
        SIGNAL(valueChanged()),
        SLOT(updateLogSizeLimit()));
    iLineChangedSignal.append(&FsIoLogModel::line0Changed);
    iLineChangedSignal.append(&FsIoLogModel::line1Changed);
    iLineChangedSignal.append(&FsIoLogModel::line2Changed);
    iLineChangedSignal.append(&FsIoLogModel::line3Changed);
    iLineChangedSignal.append(&FsIoLogModel::line4Changed);
    iLineChangedSignal.append(&FsIoLogModel::line5Changed);
    iLineChangedSignal.append(&FsIoLogModel::line6Changed);
    iLineChangedSignal.append(&FsIoLogModel::line7Changed);
    iLineChangedSignal.append(&FsIoLogModel::line8Changed);
    iLineChangedSignal.append(&FsIoLogModel::line9Changed);
    iTempDir.setAutoRemove(true);
    LOG("Temporary directory" << iTempDir.path());
    if (!iLogFile.open(QFile::Text | QFile::ReadWrite)) {
        append(QString("Failed to open ").append(iLogFile.fileName()));
    }
}

FsIoLogModel::~FsIoLogModel()
{
    deleteAllMessages();
    if (iPid > 0) kill(iPid, SIGKILL);
    if (!iArchivePath.isEmpty()) unlink(qPrintable(iArchivePath));
}

void FsIoLogModel::deleteAllMessages()
{
    qDeleteAll(iMessages);
    iMessages.clear();
}

QHash<int,QByteArray> FsIoLogModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TimeRole] = "timestamp";
    roles[TextRole] = "plaintext";
    return roles;
}

int FsIoLogModel::rowCount(const QModelIndex&) const
{
    // Leave the last row empty
    return iMessages.count() + 1;
}

QVariant FsIoLogModel::data(const QModelIndex& index, int role) const
{
    QVariant value;
    const int row = index.row();
    if (row >= 0) {
        const int count = iMessages.count();
        if (row < count) {
            const Entry* entry = iMessages.at(row);
            switch (role) {
            case TimeRole:
                value = entry->iTimestamp;
                break;
            case TextRole:
                value = entry->iText;
                break;
            default:
                break;
            }
        } else if (row == count) {
            // Last row is empty
            switch (role) {
            case TimeRole:
            case TextRole:
                value = QString();
                break;
            default:
                break;
            }
        }
    }
    return value;
}

QString FsIoLogModel::line0() const { return line(0); }
QString FsIoLogModel::line1() const { return line(1); }
QString FsIoLogModel::line2() const { return line(2); }
QString FsIoLogModel::line3() const { return line(3); }
QString FsIoLogModel::line4() const { return line(4); }
QString FsIoLogModel::line5() const { return line(5); }
QString FsIoLogModel::line6() const { return line(6); }
QString FsIoLogModel::line7() const { return line(7); }
QString FsIoLogModel::line8() const { return line(8); }
QString FsIoLogModel::line9() const { return line(9); }
QString FsIoLogModel::line(int aIndex) const
{
    if (aIndex >= 0) {
        const int count = iMessages.count();
        if (aIndex < count) {
            QString text = iMessages.at(count-aIndex-1)->iText;
            if (text.startsWith("[mms-")) {
                if (text.startsWith("[mms-task-")) {
                    text.remove(1, 9);
                } else {
                    text.remove(1, 4);
                }
            }
            return text;
        }
    }
    return QString();
}

bool FsIoLogModel::removeExtraLines(int aReserve)
{
    const int count = iMessages.count();
    if (iLogSizeLimit > 0 && (count + aReserve) > iLogSizeLimit) {
        const int nremove = (count - iLogSizeLimit) + iLogRemoveCount;
        LOG("removing last" << nremove << "lines");
        beginRemoveRows(QModelIndex(), 0, nremove-1);
        for (int i=0; i<nremove; i++) delete iMessages.at(i);
        iMessages.erase(iMessages.begin(), iMessages.begin()+nremove);
        endRemoveRows();
        return true;
    } else {
        return false;
    }
}

void FsIoLogModel::append(QString aMessage, bool aMmsEngineLog)
{
    if (aMmsEngineLog) {
        iLogFile.write(aMessage.toUtf8());
        iLogFile.write("\n");
    }
    removeExtraLines(1);
    const int count = iMessages.count();
    beginInsertRows(QModelIndex(), count+1, count+1);
    iMessages.append(new Entry(aMessage, aMmsEngineLog));
    endInsertRows();
    emit dataChanged(createIndex(count, 0), createIndex(count, 1));
    const int n = qMin(iLineChangedSignal.count(), iMessages.count());
    for (int i=0; i<n; i++) {
        (this->*iLineChangedSignal.at(i))(line(i));
    }
}

void FsIoLogModel::clear()
{
    iLogFile.flush();
    beginResetModel();
    const int n = qMin(iLineChangedSignal.count(), iMessages.count());
    deleteAllMessages();
    // line0Changed() will be signalled by append("Log cleared")
    for (int i=1; i<n; i++) {
        (this->*iLineChangedSignal.at(i))(QString());
    }
    append("Log cleared");
    endResetModel();
}

void FsIoLogModel::flush()
{
    LOG("flush");
    iLogFile.flush();
}

bool FsIoLogModel::packing() const
{
    return iPid > 0;
}

QString FsIoLogModel::archivePath() const
{
    return iArchivePath;
}

QString FsIoLogModel::archiveType() const
{
    return iArchiveType;
}

void FsIoLogModel::pack()
{
    (iArchivePath = "/tmp/mms").
        append(QDateTime::currentDateTime().toString(Qt::ISODate).replace(":","")).
        append(".tar.gz");
    LOG("Creating" << iArchivePath);
    iLogFile.flush();
    if (iPid > 0) kill(iPid, SIGKILL);
    iPid = fork();
    if (iPid > 0) {
        // Parent
        packingChanged();
    } else if (iPid == 0) {
        // Child
        execlp("tar", "tar", "-czf", qPrintable(iArchivePath), "-C",
            qPrintable(iTempDir.path()), ".", NULL);
    }
    archivePathChanged();
}

void FsIoLogModel::processDied(int aPid, int aStatus)
{
    if (iPid > 0 && iPid == aPid) {
        LOG("Tar done, pid" << aPid <<", status" << aStatus);
        iPid = -1;
        packingChanged();
    }
}

void FsIoLogModel::updateLogSizeLimit()
{
    iLogSizeLimit = LOG_SIZE_LIMIT_DEFAULT;
    iLogRemoveCount = LOG_REMOVE_DEFAULT;
    QVariant value = iLogSizeLimitConf->value();
    if (value.isValid()) {
        bool ok = false;
        int ival = value.toInt(&ok);
        if (ok) {
            if (ival <= 0) {
                iLogSizeLimit = LOG_SIZE_LIMIT_NONE;
            } else if (ival < LOG_SIZE_LIMIT_MIN) {
                iLogSizeLimit = LOG_SIZE_LIMIT_MIN;
            } else {
                iLogSizeLimit = ival;
            }
            if (iLogSizeLimit > 0) {
                iLogRemoveCount = iLogSizeLimit/50;
                if (iLogRemoveCount < 1) {
                    iLogRemoveCount = 1;
                } else if (iLogRemoveCount > LOG_REMOVE_MAX) {
                    iLogRemoveCount = LOG_REMOVE_MAX;
                }
            }
        }
    }
    LOG("log size limit" << iLogSizeLimit);
    if (removeExtraLines(0)) {
        const int count = iMessages.count();
        emit dataChanged(createIndex(count, 0), createIndex(count, 1));
    }
}
