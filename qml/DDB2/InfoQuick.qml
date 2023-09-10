import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0


DBWidget {
    friendlyName: qsTr("Info")
    internalName: "infoQuick"
    widgetStyle: "DeaDBeeF"
    widgetType: "main"
    Layout.fillWidth: true

    clip: true
    widget: Repeater {
        id: reap
        model: DBApi.playlist.current_item // Only 1 item
        Item {
            Layout.fillWidth: true
            implicitWidth: Math.max(label_title.implicitWidth, label_artist.implicitWidth)
            anchors.fill: parent
            ColumnLayout {
                width: parent.width
                //anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2
                anchors.leftMargin: 4
                Label {
                    id: label_title
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    text: ItemTitle
                    font.pixelSize: 14
                    font.bold: true
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    Layout.fillWidth: true
                }
                Label {
                    id: label_artist
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    text: ItemArtist
                    font.pixelSize: 12
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    Layout.fillWidth: true
                }
            }
        }
    }
}

/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
