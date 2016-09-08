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

#include "appsettings.h"

#include <MGConfItem>

// Both harbour and openrepos apps read settings from the same place
// That's fine.
#define DCONF_KEY(key)              QString("/apps/harbour-mmslog/%1").arg(key)
#define DCONF_LOG_SIZE_LIMIT        DCONF_KEY("logSizeLimit")
#define DCONF_FONT_SIZE_ADJUSTMENT  DCONF_KEY("fontSizeAdjustment")

const int AppSettings::DEFAULT_LOG_SIZE_LIMIT = 1000;
const int AppSettings::DEFAULT_FONT_SIZE_ADJUSTMENT = 0;

AppSettings::AppSettings(QObject* aParent) : QObject(aParent),
    iLogSizeLimit(new MGConfItem(DCONF_LOG_SIZE_LIMIT, this)),
    iFontSizeAdjustment(new MGConfItem(DCONF_FONT_SIZE_ADJUSTMENT, this))
{
    connect(iLogSizeLimit, SIGNAL(valueChanged()), SIGNAL(logSizeLimitChanged()));
    connect(iFontSizeAdjustment, SIGNAL(valueChanged()), SIGNAL(fontSizeAdjustmentChanged()));
}

int AppSettings::logSizeLimit() const
{
    QVariant value = iLogSizeLimit->value();
    if (value.isValid()) {
        bool ok = false;
        int ival = value.toInt(&ok);
        if (ok) {
            return ival;
        }
    }
    return DEFAULT_LOG_SIZE_LIMIT;
}

int AppSettings::fontSizeAdjustment() const
{
    return iFontSizeAdjustment->value(DEFAULT_FONT_SIZE_ADJUSTMENT).toInt();
}
