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

#ifndef MMSLOGMODEL_H
#define MMSLOGMODEL_H

#include <QAbstractListModel>
#include <QTemporaryDir>
#include <QThreadPool>
#include <QFile>

class AppSettings;
class FsIoLogModel : public QAbstractListModel
{
    Q_OBJECT
    class Entry;
    class SaveTask;
    Q_PROPERTY(bool packing READ packing NOTIFY packingChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY savingChanged)
    Q_PROPERTY(QString archivePath READ archivePath NOTIFY archivePathChanged)
    Q_PROPERTY(QString archiveFile READ archiveFile CONSTANT)
    Q_PROPERTY(QString archiveType READ archiveType CONSTANT)

public:
    explicit FsIoLogModel(AppSettings* aSettings, QObject* aParent = NULL);
    ~FsIoLogModel();

    QString dirName() { return iRootDir; }

    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex& aParent) const;
    virtual QVariant data(const QModelIndex& aIndex, int aRole) const;

    bool packing() const;
    bool saving() const;
    QString archivePath() const;
    QString archiveFile() const;
    QString archiveType() const;

public Q_SLOTS:
    void clear();
    void flush();
    void pack();
    void save();

Q_SIGNALS:
    void packingChanged();
    void savingChanged();
    void saveFinished(bool success);
    void archivePathChanged();
    void flushed();

private Q_SLOTS:
    void warning(QString aMessage);
    void logMessage(QString aMessage);
    void processDied(int aPid, int aStatus);
    void updateLogSizeLimit();
    void onSaveTaskDone(bool aSuccess);

private:
    void deleteAllMessages();
    bool removeExtraLines(int aReserve);
    void append(QString aMessage, int aFlags);

private:
    AppSettings* iSettings;
    QThreadPool* iThreadPool;
    QList<Entry*> iMessages;
    QString iArchivePath;
    QString iArchiveType;
    QString iArchiveName;
    QString iArchiveFile;
    QTemporaryDir iTempDir;
    QString iRootDir;
    QFile iLogFile;
    SaveTask* iSaveTask;
    int iLogSizeLimit;
    int iLogRemoveCount;
    int iPid;
};

#endif // MMSLOGMODEL_H
