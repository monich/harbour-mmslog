/*
  Copyright (C) 2014-2021 Jolla Ltd.
  Copyright (C) 2014-2021 Slava Monich <slava.monich@jolla.com>

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
import org.nemomobile.notifications 1.0

Page {
    function packAndShare() {
        FsIoLog.pack()
        pageStack.push(Qt.resolvedUrl("SharePage.qml"), {
            allowedOrientations: allowedOrientations,
        })
    }

    Notification {
        id: clipboardNotification

        //: Pop-up notification
        //% "Copied to clipboard"
        previewBody: qsTrId("mmslog-notification-copied_to_clipboard")
        expireTimeout: 2000
        Component.onCompleted: {
            if ("icon" in clipboardNotification) {
                clipboardNotification.icon = "icon-s-clipboard"
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            id: pullDownMenu
            MenuItem {
                //: Pulley menu item
                //% "Settings"
                text: qsTrId("mmslog-logpage-pm-settings")
                visible: AppSettingsMenu
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../settings/SettingsPage.qml"), {
                        title : text,
                        allowedOrientations: allowedOrientations,
                        inApp: true
                    })
                }
            }
            MenuItem {
                //: Pulley menu item
                //% "Clear log"
                text: qsTrId("mmslog-logpage-pm-clear-log")
                onClicked: FsIoLog.clear()
            }
            MenuItem {
                //: Pulley menu item
                //% "Pack and send"
                text: qsTrId("mmslog-logpage-pm-pack-and-send")
                onClicked: packAndShare()
            }
        }

        PushUpMenu {
            id: pushUpMenu
            MenuItem {
                //: Pulley menu item
                //% "Pack and send"
                text: qsTrId("mmslog-logpage-pm-pack-and-send")
                onClicked: packAndShare()
            }
            MenuItem {
                //: Pulley menu item
                //% "Clear log"
                text: qsTrId("mmslog-logpage-pm-clear-log")
                onClicked: FsIoLog.clear()
            }
        }

        SilicaListView {
            id: view
            model: FsIoLog
            anchors.fill: parent

            readonly property int rawTextSize: Theme.fontSizeTiny + AppSettings.fontSizeAdjustment
            readonly property int textSize: Math.min(Math.max(Theme.fontSizeTiny, rawTextSize), Theme.fontSizeHuge)

            header: PageHeader {
                //% "MMS engine log"
                title: qsTrId("mmslog-logpage-title")
            }
            delegate: BackgroundItem {
                width: parent.width
                height: textLabel.height
                enabled: timeLabel.text.length > 0 || textLabel.text.length > 0
                Label {
                    id: timeLabel
                    text: timestamp
                    color: model.highlight ? Theme.highlightColor : Theme.primaryColor
                    font {
                        pixelSize: view.textSize
                        bold: model.highlight
                    }
                    anchors {
                        top: parent.top
                        left: parent.left
                        leftMargin: Theme.paddingLarge
                        rightMargin: Theme.paddingLarge
                    }
                }
                Label {
                    id: textLabel
                    text: plaintext
                    wrapMode: Text.WordWrap
                    color: model.highlight ? Theme.highlightColor : Theme.primaryColor
                    font {
                        pixelSize: view.textSize
                        bold: model.highlight
                    }
                    anchors {
                        top: parent.top
                        left: timeLabel.right
                        right: parent.right
                        leftMargin: Theme.paddingLarge
                        rightMargin: Theme.paddingLarge
                    }
                }
                onPressAndHold: {
                    Clipboard.text = (timeLabel.text.length > 0) ?
                        (timeLabel.text + " " + textLabel.text) : textLabel.text
                    clipboardNotification.publish()
                }
            }

            onCountChanged: {
                if (view.atYEnd && !view.dragging && !view.flicking) {
                    positioner.restart()
                }
            }

            onHeightChanged: if (view.atYEnd) positioner.restart()

            VerticalScrollDecorator {}
        }
    }

    Timer {
        id: positioner
        interval: 100
        onTriggered: {
            if (!pullDownMenu.active && !pushUpMenu.active) {
                view.cancelFlick()
                view.positionViewAtEnd()
            }
        }
    }
}
