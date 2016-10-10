/*
 * Copyright (C) 2016 Jolla Ltd.
 * Contact: Slava Monich <slava.monich@jolla.com>
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
 *   3. Neither the name of Jolla Ltd nor the names of its contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
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

#ifndef APP_SETTINGS_H
#define APP_SETTINGS_H

#include <QtQml>

class MGConfItem;
class AppSettings : public QObject
{
    Q_OBJECT
    Q_ENUMS(OfonoLogType)
    Q_PROPERTY(int logSizeLimit READ logSizeLimit NOTIFY logSizeLimitChanged)
    Q_PROPERTY(int fontSizeAdjustment READ fontSizeAdjustment NOTIFY fontSizeAdjustmentChanged)
    Q_PROPERTY(int ofonoLogType READ ofonoLogType NOTIFY ofonoLogTypeChanged)

public:
    enum OfonoLogType {
        OfonoLogOff,
        OfonoLogMinimal,
        OfonoLogNormal,
        OfonoLogVerbose,
        OfonoLogFull,
        OfonoLogTypes
    };

    explicit AppSettings(QObject* aParent = NULL);

    static const int DEFAULT_LOG_SIZE_LIMIT;
    static const int DEFAULT_FONT_SIZE_ADJUSTMENT;
    static const OfonoLogType DEFAULT_OFONO_LOG_TYPE;

    int logSizeLimit() const;
    int fontSizeAdjustment() const;
    OfonoLogType ofonoLogType() const;

Q_SIGNALS:
    void logSizeLimitChanged();
    void fontSizeAdjustmentChanged();
    void ofonoLogTypeChanged();

private:
    MGConfItem* iLogSizeLimit;
    MGConfItem* iFontSizeAdjustment;
    MGConfItem* iOfonoLogType;
};

QML_DECLARE_TYPE(AppSettings)

#endif // APP_SETTINGS_H
