// SPDX-FileCopyrightText: 2014-2026 Slava Monich <slava.monich@jolla.com>
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
// SPDX-FileCopyrightText: 2014-2021 Jolla Ltd.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MMSLOGMODEL_H
#define MMSLOGMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QTemporaryDir>
#include <QtCore/QThreadPool>
#include <QtCore/QUrl>

class AppSettings;

class FsIoLogModel :
    public QAbstractListModel
{
    Q_OBJECT
    class Entry;
    class SaveTask;
    Q_PROPERTY(bool packing READ packing NOTIFY packingChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY savingChanged)
    Q_PROPERTY(QString archiveFile READ archiveFile CONSTANT)
    Q_PROPERTY(QString archiveType READ archiveType CONSTANT)
    Q_PROPERTY(QString archivePath READ archivePath NOTIFY archivePathChanged)
    Q_PROPERTY(qint64 archiveSize READ archiveSize NOTIFY archiveSizeChanged)
    Q_PROPERTY(QUrl archiveUrl READ archiveUrl NOTIFY archivePathChanged)

public:
    explicit FsIoLogModel(AppSettings*, QObject* parent = NULL);
    ~FsIoLogModel();

    QString dirName() { return iRootDir; }

    QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex&) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex&, int) const  Q_DECL_OVERRIDE;

    bool packing() const { return iPid > 0; }
    bool saving() const { return iSaveTask != Q_NULLPTR; }
    QString archiveFile() const { return iArchiveFile; }
    QString archivePath() const { return iArchivePath; }
    QString archiveType() const { return iArchiveType; }
    qint64 archiveSize() const { return iArchiveSize; }
    QUrl archiveUrl() const { return QUrl::fromLocalFile(iArchivePath); }

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
    void archiveSizeChanged();
    void flushed();

private Q_SLOTS:
    void warning(QString message);
    void logMessage(QString message);
    void processDied(int pid, int status);
    void updateLogSizeLimit();
    void onSaveTaskDone(bool success);

private:
    void deleteAllMessages();
    bool removeExtraLines(int);
    void append(QString, int);

private:
    AppSettings* iSettings;
    QThreadPool* iThreadPool;
    QList<Entry*> iMessages;
    QString iArchivePath;
    QString iArchiveType;
    QString iArchiveName;
    QString iArchiveFile;
    qint64 iArchiveSize;
    QTemporaryDir iTempDir;
    QString iRootDir;
    QFile iLogFile;
    SaveTask* iSaveTask;
    int iLogSizeLimit;
    int iLogRemoveCount;
    int iPid;
};

#endif // MMSLOGMODEL_H
