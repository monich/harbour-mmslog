/*
  Copyright (C) 2015-2017 Jolla Ltd.
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
    * Neither the name of Jolla Ltd nor the names of its contributors may be
      used to endorse or promote products derived from this software without
      specific prior written permission.

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

#include "ofonologger.h"
#include "appsettings.h"
#include "manager_interface.h"
#include "mmsdebug.h"

#include <fcntl.h>
#include <unistd.h>

#include <QMutex>
#include <QWaitCondition>

#include "gutil_ring.h"
#include "gutil_strv.h"

#undef signals
#include "dbuslog_client.h"

#define OFONO_BUS QDBusConnection::systemBus()
#define OFONO_SERVICE "org.ofono"
#define OFONO_INTERFACE OrgOfonoManager::staticInterfaceName()

// Forward declarations

class OfonoLogger::Private: public QObject
{
    Q_OBJECT
    class Capture;

public:
    Private(QString aDir, AppSettings* aSettings, OfonoLogger* aParent);

    void saveInfo(QString aModemPath, const char* aCall);

private Q_SLOTS:
    void onGetModemsFinished(QDBusPendingCallWatcher* aCall);
    void onOfonoLogTypeChanged();

public:
    QString iDir;
    QString iLogFile;
    AppSettings* iSettings;
    OrgOfonoManager* iOfonoManager;
    Capture* iCapture;
};

// ==========================================================================
// OfonoLogger::Private::Capture
// ==========================================================================

class OfonoLogger::Private::Capture: public QThread {
    Q_OBJECT

    enum LogEvent {
        LogEventConnect,
        LogEventMessage,
        LogEventCount
    };

    struct LogEntry {
        DBusLogCategory* cat;
        DBusLogMessage* msg;
    };

public:
    Capture(QString aPath, AppSettings::OfonoLogType aType, QObject* aParent);
    ~Capture();

    virtual void run();

    static void freeLogEntry(gpointer aData);
    static void logConnect(DBusLogClient* aClient, gpointer aData);
    static void logMessage(DBusLogClient* aClient, DBusLogCategory* aCategory,
        DBusLogMessage* aMessage, gpointer aData);

    void setLogType(AppSettings::OfonoLogType aType);
    void checkDefaultLogLevel();
    void flush();

private Q_SLOTS:
    void onLogConnect();
    void onLogMessage(DBusLogCategory* aCategory, DBusLogMessage* aMessage);

public:
    QString iPath;
    AppSettings::OfonoLogType iLogType;
    GDBusConnection* iConnection;
    DBusLogClient* iClient;
    gulong iEventId[LogEventCount];
    GStrV* iEnabledCategories[AppSettings::OfonoLogTypes];
    GUtilRing* iMessageRing;
    QWaitCondition iWaitCondition;
    QMutex iMutex;
    bool iFlush;
    bool iStarted;
    bool iExiting;
};

OfonoLogger::Private::Capture::Capture(
    QString aPath,
    AppSettings::OfonoLogType aType,
    QObject* aParent) :
    QThread(aParent),
    iPath(aPath),
    iLogType(aType),
    iConnection(g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL)),
    iClient(dbus_log_client_new(G_BUS_TYPE_SYSTEM, OFONO_SERVICE, "/",
        DBUSLOG_CLIENT_FLAG_AUTOSTART)),
    iMessageRing(NULL),
    iFlush(false),
    iStarted(false),
    iExiting(false)
{
    LOG("initializing ofono logging");
    memset(iEnabledCategories, 0, sizeof(iEnabledCategories));
    iEventId[LogEventConnect] =
        dbus_log_client_add_connected_handler(iClient, logConnect, this);
    iEventId[LogEventMessage] =
        dbus_log_client_add_message_handler(iClient, logMessage, this);
    onLogConnect();
}

OfonoLogger::Private::Capture::~Capture()
{
    bool flush = false;
    dbus_log_client_remove_handlers(iClient, iEventId, LogEventCount);
    for (int type = 0; type < AppSettings::OfonoLogTypes; type++) {
        GStrV* cats = iEnabledCategories[type];
        if (cats) {
            if (type <= iLogType) {
                LOG("disabling" << gutil_strv_length(cats) << "categories");
                dbus_log_client_disable_categories(iClient, cats, NULL, NULL);
            }
            g_strfreev(cats);
            flush = true;
            iEnabledCategories[type] = NULL;
        }
    }
    if (iStarted) {
        LOG("stopping ofono log thread");
        iMutex.lock();
        iExiting = true;
        iWaitCondition.wakeAll();
        iMutex.unlock();
        wait();
    }
    if (flush) g_dbus_connection_flush_sync(iConnection, NULL, NULL);
    dbus_log_client_unref(iClient);
    g_object_unref(iConnection);
    gutil_ring_unref(iMessageRing);
}

void
OfonoLogger::Private::Capture::freeLogEntry(
    gpointer aData)
{
    LogEntry* entry = (LogEntry*)aData;
    dbus_log_category_unref(entry->cat);
    dbus_log_message_unref(entry->msg);
    g_slice_free(LogEntry, entry);
}

void
OfonoLogger::Private::Capture::logConnect(
    DBusLogClient* aClient,
    gpointer aData)
{
    // Why invokeMethod? See https://bugreports.qt.io/browse/QTBUG-18434
    QMetaObject::invokeMethod((Capture*)aData, "onLogConnect");
}

void
OfonoLogger::Private::Capture::logMessage(
    DBusLogClient* aClient,
    DBusLogCategory* aCategory,
    DBusLogMessage* aMessage,
    gpointer aData)
{
    // Why invokeMethod? See https://bugreports.qt.io/browse/QTBUG-18434
    QMetaObject::invokeMethod((Capture*)aData, "onLogMessage",
        Q_ARG(DBusLogCategory*,aCategory),
        Q_ARG(DBusLogMessage*,aMessage));
}

void
OfonoLogger::Private::Capture::checkDefaultLogLevel()
{
    if (iLogType >= AppSettings::OfonoLogFull) {
        if (iClient->default_level < DBUSLOG_LEVEL_VERBOSE) {
            dbus_log_client_set_default_level(iClient,
                DBUSLOG_LEVEL_VERBOSE, NULL, NULL);
        }
    }
}

void
OfonoLogger::Private::Capture::onLogConnect()
{
    if (!iMessageRing && iClient->connected) {
        iMessageRing = gutil_ring_new();
        gutil_ring_set_free_func(iMessageRing, freeLogEntry);
        // Store the names of disabled categories and enable them. Keep
        // the list so that we disable them when we shut down.
        for (guint i=0; i<iClient->categories->len; i++) {
            const DBusLogCategory* cat = (DBusLogCategory*)
                iClient->categories->pdata[i];
            if (!(cat->flags & DBUSLOG_CATEGORY_FLAG_ENABLED)) {
                AppSettings::OfonoLogType type;
                if (!strcmp(cat->name, "ril_dump")) {
                    type = AppSettings::OfonoLogFull;
                } else {
                    // Well, we could be more selective...
                    type = AppSettings::OfonoLogMinimal;
                }
                iEnabledCategories[type] =
                    gutil_strv_add(iEnabledCategories[type], cat->name);
            }
        }
        checkDefaultLogLevel();
        for (int type = 0; type <= iLogType; type++) {
            GStrV* cats = iEnabledCategories[type];
            if (cats) {
                LOG("enabling" << gutil_strv_length(cats) << "categories");
                dbus_log_client_enable_categories(iClient, cats, NULL, NULL);
            }
        }
        LOG("starting ofono log thread");
        iStarted = true;
        start();
    }
}

void
OfonoLogger::Private::Capture::setLogType(
    AppSettings::OfonoLogType aType)
{
    if (iClient) {
        if (iLogType > aType) {
            // Log level has been reduced
            for (int type = iLogType; type > aType; type--) {
                GStrV* cats = iEnabledCategories[type];
                if (cats) {
                    LOG("disabling" << gutil_strv_length(cats) << "categories");
                    dbus_log_client_disable_categories(iClient, cats, NULL, NULL);
                }
            }
        } else if (iLogType < aType) {
            // Log level has been increased
            checkDefaultLogLevel();
            for (int type = iLogType+1; type <= aType; type++) {
                GStrV* cats = iEnabledCategories[type];
                if (cats) {
                    LOG("enabling" << gutil_strv_length(cats) << "categories");
                    dbus_log_client_enable_categories(iClient, cats, NULL, NULL);
                }
            }
        }
    }
    iLogType = aType;
}

void
OfonoLogger::Private::Capture::onLogMessage(
    DBusLogCategory* aCategory,
    DBusLogMessage* aMessage)
{
    if (iMessageRing) {
        LogEntry* entry = g_slice_new(LogEntry);
        entry->cat = dbus_log_category_ref(aCategory);
        entry->msg = dbus_log_message_ref(aMessage);
        iMutex.lock();
        gutil_ring_put(iMessageRing, entry);
        iWaitCondition.wakeAll();
        iMutex.unlock();
    }
}

void
OfonoLogger::Private::Capture::run()
{
    LOG("ofono log thread started");
    QFile file(iPath);
    if (file.open(QFile::Text | QFile::ReadWrite)) {
        DBusLogMessage* last = NULL;
        iMutex.lock();
        while (!iExiting) {
            LogEntry* entry;
            bool flush = false;
            while (!iExiting &&
                !(entry = (LogEntry*)gutil_ring_get(iMessageRing)) &&
                !iFlush) {
                iWaitCondition.wait(&iMutex);
            }
            flush = iFlush;
            iFlush = false;
            iMutex.unlock();
            if (entry) {
                DBusLogCategory* cat = entry->cat;
                DBusLogMessage* msg = entry->msg;

                // See if we skipped anything
                if (last && (last->index + 1 != msg->index)) {
                    const guint32 skip = msg->index - last->index - 1;
                    file.write(QString("... skipped %1 entries\n").
                        arg(skip).toUtf8());
                }

                // Write this message
                QString line;
                QDateTime t(QDateTime::fromMSecsSinceEpoch(msg->timestamp/1000));
                static const QString TIME_FORMAT("yyyy-MM-dd hh:mm:ss.zzz ");
                QString timestamp(t.toString(TIME_FORMAT));
                QString text(QString::fromUtf8(msg->string, msg->length));
                if (cat && !(cat->flags & DBUSLOG_CATEGORY_FLAG_HIDE_NAME)) {
                    static const QString FORMAT1("%1%2: %3\n");
                    QString cname(QString::fromUtf8(cat->name));
                    line = FORMAT1.arg(timestamp, cname, text);
                } else{
                    static const QString FORMAT2("%1%2\n");
                    line = FORMAT2.arg(timestamp, text);
                }
                file.write(line.toUtf8());
                dbus_log_category_unref(cat);
                dbus_log_message_unref(last);
                g_slice_free(LogEntry, entry);
                last = msg;
            }
            if (flush) {
                LOG("flushing ofono log");
                file.flush();
            }
            iMutex.lock();
        }
        iMutex.unlock();
        file.close();
        dbus_log_message_unref(last);
    } else {
        WARN("Failed to open " << qPrintable(file.fileName()));
    }
    LOG("ofono log thread exiting");
}

void
OfonoLogger::Private::Capture::flush()
{
    if (iStarted) {
        iMutex.lock();
        iFlush = true;
        iWaitCondition.wakeAll();
        iMutex.unlock();
    }
}

// ==========================================================================
// OfonoLogger::Private
// ==========================================================================

OfonoLogger::Private::Private(
    QString aDir,
    AppSettings* aSettings,
    OfonoLogger* aParent) :
    QObject(aParent),
    iDir(aDir),
    iLogFile(aDir + "/ofono.log"),
    iSettings(aSettings),
    iOfonoManager(new OrgOfonoManager(OFONO_SERVICE, "/", OFONO_BUS, this)),
    iCapture(NULL)
{
    connect(new QDBusPendingCallWatcher(iOfonoManager->GetModems(), this),
        SIGNAL(finished(QDBusPendingCallWatcher*)),
        SLOT(onGetModemsFinished(QDBusPendingCallWatcher*)));
    connect(iSettings, SIGNAL(ofonoLogTypeChanged()), SLOT(onOfonoLogTypeChanged()));
    onOfonoLogTypeChanged();
}

void
OfonoLogger::Private::onOfonoLogTypeChanged()
{
    const AppSettings::OfonoLogType logType = iSettings->ofonoLogType();
    LOG(logType);
    if (logType == AppSettings::OfonoLogOff) {
        if (iCapture) {
            delete iCapture;
            iCapture = NULL;
            if (QFile::remove(iLogFile)) {
                LOG("removed" << qPrintable(iLogFile));
            }
        }
    } else {
        if (iCapture) {
            iCapture->setLogType(logType);
        } else {
            iCapture = new Capture(iLogFile, logType, this);
        }
    }
}

void
OfonoLogger::Private::onGetModemsFinished(
    QDBusPendingCallWatcher* aCall)
{
    QDBusPendingReply<OfonoObjectPathPropertiesList> reply(*aCall);
    aCall->deleteLater();
    if (reply.isError()) {
        qWarning() << reply.error().message();
    } else {
        OfonoObjectPathPropertiesList list = reply.value();
        const int n = list.count();
        for (int i=0; i<n; i++) {
            QString modem(list[i].path.path());
            saveInfo(modem, "SimManager.GetProperties");
            saveInfo(modem, "ConnectionManager.GetContexts");
            saveInfo(modem, "NetworkRegistration.GetProperties");
        }
    }
}

void
OfonoLogger::Private::saveInfo(
    QString aModemPath,
    const char* aCall)
{
    QString call(OFONO_INTERFACE);
    call += ".";
    call += aCall;

    QString file(iDir);
    file += "/";
    file += aCall;
    file += QString(aModemPath).replace('/', '.');
    file += ".txt";

    int fd = open(qPrintable(file), O_WRONLY | O_CREAT, 0644);
    if (fd >= 0) {
        if (fork() == 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            execlp("dbus-send", "dbus-send", "--system", "--print-reply",
                "--dest=" OFONO_SERVICE, qPrintable(aModemPath),
                qPrintable(call), NULL);
        }
        close(fd);
    }
}

// ==========================================================================
// OfonoLogger
// ==========================================================================

OfonoLogger::OfonoLogger(
    QString aDir,
    AppSettings* aSettings,
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(aDir, aSettings, this))
{
}

void
OfonoLogger::registerType()
{
    qDBusRegisterMetaType<OfonoObjectPathProperties>();
    qDBusRegisterMetaType<OfonoObjectPathPropertiesList>();
}

void
OfonoLogger::flush()
{
    if (iPrivate->iCapture) {
        iPrivate->iCapture->flush();
    }
}

QDBusArgument&
operator<<(
    QDBusArgument& aArg,
    const OfonoObjectPathProperties& aList)
{
    aArg.beginStructure();
    aArg << aList.path << aList.properties;
    aArg.endStructure();
    return aArg;
}

const QDBusArgument&
operator>>(
    const QDBusArgument& aArg,
    OfonoObjectPathProperties& aList)
{
    aArg.beginStructure();
    aArg >> aList.path >> aList.properties;
    aArg.endStructure();
    return aArg;
}

#include "ofonologger.moc"
