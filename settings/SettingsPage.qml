/*
  Copyright (C) 2015-2021 Jolla Ltd.
  Copyright (C) 2015-2021 Slava Monich <slava.monich@jolla.com>

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the names of the copyright holders nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

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

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0

Page {
    readonly property string rootPath: "/apps/harbour-mmslog/"
    property alias title: pageHeader.title
    property bool inApp

    // jolla-settings expects these properties:
    property var applicationName
    property var applicationIcon

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            PageHeader {
                id: pageHeader
                rightMargin: Theme.horizontalPageMargin + (appIcon.visible ? (height - appIcon.padding) : 0)
                title: applicationName ? applicationName :
                    //: Settings page title (app name)
                    //% "MMS Logger"
                    qsTrId("mmslog-settings-page-header")
                description: inApp ? "" :
                    //: Settings page header description (app version)
                    //% "Version %1"
                    qsTrId("mmslog-settings-version").arg("1.0.18")

                Image {
                    id: appIcon
                    readonly property int padding: Theme.paddingLarge
                    readonly property int size: pageHeader.height - 2 * padding
                    x: pageHeader.width - width - Theme.horizontalPageMargin
                    y: padding
                    width: size
                    height: size
                    sourceSize: Qt.size(size,size)
                    source: applicationIcon ? applicationIcon : ""
                    visible: appIcon.status === Image.Ready
                }
            }

            Slider {
                id: fontSizeSlider
                width: parent.width
                minimumValue: Theme.fontSizeTiny
                maximumValue: Theme.fontSizeLarge
                stepSize: 1
                //% "Font size"
                label: qsTrId("mmslog-settings-fontsize-label")
                valueText: minimumValue + sliderValue
                onSliderValueChanged: fontSizeAdjustment.value = sliderValue - minimumValue
                Component.onCompleted: value = minimumValue + fontSizeAdjustment.value

                ConfigurationValue {
                    id: fontSizeAdjustment
                    key: rootPath + "fontSizeAdjustment"
                    onValueChanged: fontSizeSlider.value = fontSizeSlider.minimumValue + value
                    defaultValue: 0
                }
            }

            SettingsComboBox {
                key: rootPath + "logSizeLimit"
                //% "Screen buffer size"
                label: qsTrId("mmslog-settings-logsizelimit")
                //% "Don't worry, everything will be written to the log file regardless of the screen buffer size."
                description: qsTrId("mmslog-settings-logsizelimit-description")
                menu: ContextMenu {
                    readonly property int defaultIndex: 1
                    MenuItem {
                        text: value
                        readonly property int value: 100
                    }
                    MenuItem {
                        text: value
                        readonly property int value: 1000
                    }
                    MenuItem {
                        text: value
                        readonly property int value: 10000
                    }
                    MenuItem {
                        //% "Unlimited"
                        text: qsTrId("mmslog-settings-logsizelimit-unlimited")
                        readonly property int value: 0
                    }
                }
            }

            SettingsComboBox {
                key: rootPath + "ofonoLogType"
                //% "Ofono log"
                label: qsTrId("mmslog-settings-ofonolog-label")
                //% "Ofono log provides additional information on what's happening at telephony level. Currently this log is not visible in the user interface but if enabled, it's packed and stored in the tarball."
                description: qsTrId("mmslog-settings-ofonolog-description")
                menu: ContextMenu {
                    readonly property int defaultIndex: 1
                    MenuItem {
                        //% "Off"
                        text: qsTrId("mmslog-settings-ofonolog-value-off")
                        readonly property int value: 0
                    }
                    MenuItem {
                        //% "Normal"
                        text: qsTrId("mmslog-settings-ofonolog-value-normal")
                        readonly property int value: 2
                    }
                    MenuItem {
                        //% "Full"
                        text: qsTrId("mmslog-settings-ofonolog-value-full")
                        readonly property int value: 4
                    }
                }
            }
        }
    }
}
