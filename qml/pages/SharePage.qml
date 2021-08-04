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

import "../harbour"

Page {
    id: page

    allowedOrientations: window.allowedOrientations
    // backNavigation has to be true when the page is pushed to the page stack
    // so that the right animation is used (consistent with the settings page)
    backNavigation: _readyToShare || (status === PageStatus.Activating || status === PageStatus.Inactive)
    // Don't show the back indicator while the push animation is active
    showNavigationIndicator: _readyToShare && (status === PageStatus.Active || status === PageStatus.Deactivating)
    readonly property string _sharingApiVersion: SystemInfo.packageVersion("declarative-transferengine-qt5")
    readonly property bool _sharingBroken: SystemInfo.compareVersions(_sharingApiVersion, "0.4.0") >= 0 // QML API break
    property bool _readyToShare: !FsIoLog.packing && !FsIoLog.saving && !minWaitTimer.running
    property bool _portrait: page.orientation === Orientation.Portrait
    property real _fullHeight: _portrait ? window.height : window.width

    // The timer makes sure that animation is displayed for at least 1 second
    Timer {
        id: minWaitTimer
        interval: 1000
        running: true
    }

    Notification {
        id: notification
    }

    Connections {
        target: FsIoLog
        onSaveFinished: {
            notification.close()
            if (success) {
                //% "Saved %1"
                notification.previewBody = qsTrId("mmslog-sharepage-save-ok").arg(FsIoLog.archiveFile)
            } else {
                //% "Failed to save %1"
                notification.previewBody = qsTrId("mmslog-sharepage-save-error").arg(FsIoLog.archiveFile)
            }
            notification.publish()
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        PullDownMenu {
            visible: _readyToShare || active
            MenuItem {
                //% "Save to documents"
                text: qsTrId("mmslog-sharepage-pm-save-to-documents")
                onClicked: FsIoLog.save()
            }
            onActiveChanged: {
                if (!active && FsIoLog.saving) {
                    // Copying hasn't finished by the time menu was closed
                    minWaitTimer.start()
                }
            }
        }

        PageHeader {
            id: header
            //% "Pack and send"
            title: qsTrId("mmslog-sharepage-header")
        }

        Item {
            id: shareMethods
            anchors {
                top: header.bottom
                left: parent.left
            }
            width: parent.width
            visible: opacity > 0
            opacity: _readyToShare
            Behavior on opacity { FadeAnimation {} }

            Loader {
                active: _sharingBroken
                anchors.fill: parent
                sourceComponent: Component {
                    Item {
                        anchors.fill: parent

                        InfoLabel {
                            id: sharingBrokenInfo
                            //: Info label displayed instead of sharing method list
                            //% "In-app sharing is not available in this version of Sailfish OS. Use the pulley menu to save tarball to the documents folder."
                            text: qsTrId("mmslog-sharepage-sharing_broken")
                        }
                        Item {
                            anchors {
                                left: parent.left
                                right: parent.right
                                top: sharingBrokenInfo.bottom
                                bottom: parent.bottom
                            }
                            HarbourHighlightIcon {
                                anchors.centerIn: parent
                                sourceSize.height: Math.min(Math.floor(parent.height/2), Theme.itemSizeSmall)
                                visible: height >= Theme.itemSizeSmall // Too small would look too stupid
                                source: "images/shrug.svg"
                            }
                        }
                    }
                }
            }

            Loader {
                active: !_sharingBroken
                anchors.fill: parent
                sourceComponent: Component {
                    HarbourShareMethodList {
                        visible: opacity > 0
                        opacity: _readyToShare
                        model: TransferMethodsModel
                        source: FsIoLog.archivePath
                        type: FsIoLog.archiveType
                        subject: "Jolla MMS log"
                        emailTo: "mms-debug@jolla.com"
                        //: Share list item
                        //% "Add account"
                        addAccountText: qsTrId("mmslog-sharemethodlist-add-account")
                        VerticalScrollDecorator {}
                    }
                }
            }
        }

        Label {
            id: warning

            visible: opacity > 0
            opacity: _readyToShare
            height: implicitHeight
            Behavior on opacity { FadeAnimation {} }
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeExtraSmall
            verticalAlignment: Text.AlignTop
            color: Theme.secondaryColor
            //% "Keep in mind that some of the information contained in this archive may be considered private. If you would like to check what you are about to send, please consider sending it to yourself first and emailing this file to mms-debug@jolla.com later from your computer. If you trust Jolla, then you can conveniently email it to Jolla directly from your phone."
            text: qsTrId("mmslog-sharepage-warning")
        }

        states: [
            State {
                name: "PORTRAIT"
                when: page.orientation === Orientation.Portrait
                AnchorChanges {
                    target: shareMethods
                    anchors {
                        right: parent.right
                        bottom: warning.top
                    }
                }
                PropertyChanges {
                    target: shareMethods
                    anchors.bottomMargin: Theme.paddingLarge
                    anchors.rightMargin: 0
                }
                PropertyChanges {
                    target: warning
                    x: Theme.horizontalPageMargin
                    y: window.height - height - Theme.paddingLarge
                    width: parent.width - 2*Theme.horizontalPageMargin
                }
            },
            State {
                name: "LANDSCAPE"
                when: page.orientation !== Orientation.Portrait
                AnchorChanges {
                    target: shareMethods
                    anchors {
                        right: warning.left
                        bottom: parent.bottom
                    }
                }
                PropertyChanges {
                    target: shareMethods
                    anchors.bottomMargin: 0
                    anchors.rightMargin: Theme.horizontalPageMargin
                }
                PropertyChanges {
                    target: warning
                    x: parent.width - width - Theme.horizontalPageMargin
                    y: header.y + header.height
                    width: parent.width*2/5 - 2*Theme.horizontalPageMargin
                }
            }
        ]
    }

    Column {
        visible: opacity > 0
        opacity: _readyToShare ? 0 : 1
        anchors.centerIn: parent
        spacing: Theme.paddingLarge
        Behavior on opacity { FadeAnimation {} }
        BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            size: BusyIndicatorSize.Large
            running: !_readyToShare
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            horizontalAlignment: Text.AlignHCenter
            color: Theme.highlightColor
            //% "Please wait"
            text: qsTrId("mmslog-sharepage-please-wait")
        }
    }
}
