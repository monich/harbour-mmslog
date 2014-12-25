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

CoverBackground {
    id: cover
    Label {
        id: title
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("mmslog-cover-title")
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            topMargin: Theme.paddingLarge
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
        }
    }
    Column {
        anchors {
            top: title.bottom
            left: parent.left
            right: parent.right
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingSmall
            topMargin: Theme.paddingSmall
        }
        CoverLogLine { text: FsIoLog.line7; opacity: 0.3 }
        CoverLogLine { text: FsIoLog.line6; opacity: 0.6 }
        CoverLogLine { text: FsIoLog.line5 }
        CoverLogLine { text: FsIoLog.line4 }
        CoverLogLine { text: FsIoLog.line3 }
        CoverLogLine { text: FsIoLog.line2 }
        CoverLogLine { text: FsIoLog.line1 }
        CoverLogLine { text: FsIoLog.line0 }
    }
    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-cover-cancel"
            onTriggered: FsIoLog.clear()
        }
    }
}
