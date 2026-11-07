// SPDX-FileCopyrightText: 2014-2026 Slava Monich <slava.monich@jolla.com>
// SPDX-FileCopyrightText: 2026 Jolla Mobile Ltd
// SPDX-FileCopyrightText: 2014-2021 Jolla Ltd.
// SPDX-License-Identifier: BSD-3-Clause

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
            if (!pullDownMenu.active) {
                view.cancelFlick()
                view.positionViewAtEnd()
            }
        }
    }
}
