/*
  Copyright (C) 2014 Jolla Ltd.
  Contact: Slava Monich <slava.monich@jolla.com>
  All rights reserved.

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

Page {
    id: page
    allowedOrientations: window.allowedOrientations
    property bool _canShare: !FsIoLog.packing && !minStartupAnimationDelay.running
    property bool _portrait: page.orientation === Orientation.Portrait

    // The timer makes sure that animation is displayed for at least 2 seconds
    Timer {
        id: minStartupAnimationDelay
        interval: 1000
        running: true
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: parent.height

        PageHeader {
            id: header
            title: "Pack and send"
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
            Behavior on opacity { FadeAnimation {} }
            anchors {
                top: _portrait ? undefined : header.bottom
                bottom: _portrait ? parent.bottom : undefined
                right: parent.right
                rightMargin: Theme.paddingLarge
                bottomMargin: Theme.paddingLarge
            }
            wrapMode: Text.WordWrap
            font.pixelSize: Theme.fontSizeExtraSmall
            verticalAlignment: Text.AlignTop
            color: Theme.secondaryColor
            text: "Keep in mind that some of the information contained \
in this archive may be considered private. If you would like to check \
what you are about to send, please consider sending it to yourself first \
and emailing this file to mms-debug@jolla.com later from your computer. \
If you trust Jolla, then you can conveniently email it to Jolla directly \
from your phone."
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
            text: "Please wait..."
        }
    }
}
