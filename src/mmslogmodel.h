/*
  Copyright (C) 2014-2016 Jolla Ltd.
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

#ifndef MMSLOGMODEL_H
#define MMSLOGMODEL_H

#include <QAbstractListModel>
#include <QTemporaryDir>
#include <QThreadPool>
#include <QFile>
#include <MGConfItem>

class FsIoLogModel : public QAbstractListModel
{
    Q_OBJECT
    class Entry;
    class SaveTask;
    typedef void (FsIoLogModel::*LineChanged)(QString);
    Q_PROPERTY(bool packing READ packing NOTIFY packingChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY savingChanged)
    Q_PROPERTY(QString archivePath READ archivePath NOTIFY archivePathChanged)
    Q_PROPERTY(QString archiveFile READ archiveFile CONSTANT)
    Q_PROPERTY(QString archiveType READ archiveType CONSTANT)
    Q_PROPERTY(QString line0 READ line0 NOTIFY line0Changed)
    Q_PROPERTY(QString line1 READ line1 NOTIFY line1Changed)
    Q_PROPERTY(QString line2 READ line2 NOTIFY line2Changed)
    Q_PROPERTY(QString line3 READ line3 NOTIFY line3Changed)
    Q_PROPERTY(QString line4 READ line4 NOTIFY line4Changed)
    Q_PROPERTY(QString line5 READ line5 NOTIFY line5Changed)
    Q_PROPERTY(QString line6 READ line6 NOTIFY line6Changed)
    Q_PROPERTY(QString line7 READ line7 NOTIFY line7Changed)
    Q_PROPERTY(QString line8 READ line8 NOTIFY line8Changed)
    Q_PROPERTY(QString line9 READ line9 NOTIFY line9Changed)

public:
    explicit FsIoLogModel(QObject* aParent = NULL);
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

    QString line0() const;
    QString line1() const;
    QString line2() const;
    QString line3() const;
    QString line4() const;
    QString line5() const;
    QString line6() const;
    QString line7() const;
    QString line8() const;
    QString line9() const;

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
    void line0Changed(QString aValue);
    void line1Changed(QString aValue);
    void line2Changed(QString aValue);
    void line3Changed(QString aValue);
    void line4Changed(QString aValue);
    void line5Changed(QString aValue);
    void line6Changed(QString aValue);
    void line7Changed(QString aValue);
    void line8Changed(QString aValue);
    void line9Changed(QString aValue);

private Q_SLOTS:
    void append(QString aMessage, bool aMmsEngineLog = false);
    void processDied(int aPid, int aStatus);
    void updateLogSizeLimit();
    void onSaveTaskDone(bool aSuccess);

private:
    QString line(int aIndex) const;
    void deleteAllMessages();
    bool removeExtraLines(int aReserve);

private:
    QThreadPool* iThreadPool;
    QList<Entry*> iMessages;
    QList<LineChanged> iLineChangedSignal;
    QString iArchivePath;
    QString iArchiveType;
    QString iArchiveName;
    QString iArchiveFile;
    QTemporaryDir iTempDir;
    QString iRootDir;
    QFile iLogFile;
    MGConfItem* iLogSizeLimitConf;
    SaveTask* iSaveTask;
    int iLogSizeLimit;
    int iLogRemoveCount;
    int iPid;
};

#endif // MMSLOGMODEL_H
