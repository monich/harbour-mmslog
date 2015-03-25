/*
  Copyright (C) 2014-2015 Jolla Ltd.
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

Page {
    id: page
    allowedOrientations: window.allowedOrientations
    property bool _clear

    // Clearing the model when pully menu is open results in unpleasant
    // visual effect. Couldn't find a better way than waiting until the
    // pully menu disappears from the screen
    function doClear() {
        _clear = false
        FsIoLog.clear()
    }

    function packAndShare() {
        FsIoLog.pack()
        pageStack.push(sharePageComponent)
    }

    Component {
        id: sharePageComponent
        SharePage {}
    }

    SilicaListView {
        id: view
        model: FsIoLog
        anchors.fill: parent

        PullDownMenu {
            id: pullDownMenu
            MenuItem {
                text: qsTr("mmslog-logpage-pm-clear-log")
                onClicked: _clear = true
            }
            MenuItem {
                text: qsTr("mmslog-logpage-pm-pack-and-send")
                onClicked: packAndShare()
            }
            onActiveChanged: if (!active && _clear) doClear()
        }

        PushUpMenu {
            id: pushUpMenu
            MenuItem {
                text: qsTr("mmslog-logpage-pm-pack-and-send")
                onClicked: packAndShare()
            }
            MenuItem {
                text: qsTr("mmslog-logpage-pm-clear-log")
                onClicked: _clear = true
            }
            onActiveChanged: if (!active && _clear) doClear()
        }

        header: PageHeader { title: qsTr("mmslog-logpage-title") }
        delegate: Item {
            width: parent.width
            height: textLabel.height
            Label {
                id: timeLabel
                text: timestamp
                font.pixelSize: Theme.fontSizeTiny
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
                font.pixelSize: Theme.fontSizeTiny
                wrapMode: Text.WordWrap
                anchors {
                    top: parent.top
                    left: timeLabel.right
                    right: parent.right
                    leftMargin: Theme.paddingLarge
                    rightMargin: Theme.paddingLarge
                }
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
