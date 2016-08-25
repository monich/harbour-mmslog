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

import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.notifications 1.0

Page {
    id: page
    allowedOrientations: window.allowedOrientations
    backNavigation: _canShare
    property bool _canShare: !FsIoLog.packing && !FsIoLog.saving && !minWaitTimer.running
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
            visible: _canShare || active
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

        ShareMethodList {
            visible: opacity > 0
            opacity: _canShare
            id: shareMethods
            source: FsIoLog.archivePath
            filter: FsIoLog.archiveType
            type: FsIoLog.archiveType
            subject: "Jolla MMS log"
            emailTo: "mms-debug@jolla.com"
            Behavior on opacity { FadeAnimation {} }
            anchors {
                top: header.bottom
                left: parent.left
                right: _portrait ? parent.right : warning.left
                rightMargin: _portrait ? 0 : Theme.paddingLarge
                bottom: _portrait ? warning.top : parent.bottom
                bottomMargin: _portrait ? Theme.paddingLarge : 0
            }
            VerticalScrollDecorator {}
        }

        Label {
            id: warning
            visible: opacity > 0
            opacity: _canShare
            width: (_portrait ? parent.width : parent.width*2/5) - 2*Theme.paddingLarge
            height: implicitHeight
            x: _portrait ? Theme.paddingLarge : (parent.width - width - Theme.paddingLarge)
            y: _portrait ? (_fullHeight - height - Theme.paddingLarge) : (header.y + header.height)
            Behavior on opacity { FadeAnimation {} }
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeExtraSmall
            verticalAlignment: Text.AlignTop
            color: Theme.secondaryColor
            //% "Keep in mind that some of the information contained in this archive may be considered private. If you would like to check what you are about to send, please consider sending it to yourself first and emailing this file to mms-debug@jolla.com later from your computer. If you trust Jolla, then you can conveniently email it to Jolla directly from your phone."
            text: qsTrId("mmslog-sharepage-warning")
        }
    }

    Column {
        visible: opacity > 0
        opacity: _canShare ? 0 : 1
        anchors.centerIn: parent
        spacing: Theme.paddingLarge
        Behavior on opacity { FadeAnimation {} }
        BusyIndicator {
            id: busy
            size: BusyIndicatorSize.Large
            running: !_canShare
        }
        Label {
            anchors.horizontalCenter: busy.horizontalCenter
            horizontalAlignment: Text.AlignHCenter
            //% "Please wait"
            text: qsTrId("mmslog-sharepage-please-wait")
        }
    }
}
