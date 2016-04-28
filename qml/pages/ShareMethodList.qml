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
import harbour.mmslog 1.0
import org.nemomobile.dbus 2.0

SilicaListView {
    id: root

    property url source
    property string subject
    property string emailTo
    property alias filter: transferMethodsModel.filter
    property alias type: content.type

    width: parent.width
    height: Theme.itemSizeSmall * transferMethodsModel.count

    QtObject {
        id: content
        property string type
    }

    DBusInterface {
        id: settings

        service: "com.jolla.settings"
        path: "/com/jolla/settings/ui"
        iface: "com.jolla.settings.ui"
    }

    model: TransferMethodsModel { id: transferMethodsModel }

    delegate: BackgroundItem {
        id: backgroundItem
        width: root.width

        Label {
            id: displayNameLabel
            text: displayName
            color: backgroundItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            truncationMode: TruncationMode.Fade
            x: Theme.paddingLarge
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(implicitWidth, parent.width - 2*Theme.paddingLarge)
        }

        Label {
            text: userName
            font.pixelSize: Theme.fontSizeMedium
            color: backgroundItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
            truncationMode: TruncationMode.Fade
            anchors {
                left: displayNameLabel.right
                leftMargin: Theme.paddingSmall
                right: parent.right
                rightMargin: Theme.paddingLarge
                verticalCenter: parent.verticalCenter
            }
            visible: text.length > 0
        }

        onClicked: {
            pageStack.push(shareUIPath, {
                source: root.source,
                content: content,
                methodId: methodId,
                displayName: displayName,
                accountId: accountId,
                accountName: userName,
                emailTo: root.emailTo,
                emailSubject: root.subject
            })
        }
    }

    footer: BackgroundItem {
        Label {
            text: qsTr("mmslog-sharemethodlist-add-account")
            x: Theme.horizontalPageMargin
            anchors.verticalCenter: parent.verticalCenter
            color: highlighted ? Theme.highlightColor : Theme.primaryColor
        }
        onClicked: {
            settings.call("showAccounts", undefined)
        }
    }
}
