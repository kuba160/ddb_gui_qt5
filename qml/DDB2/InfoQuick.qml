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

    clip: true
    widget: Repeater {
        id: reap
        model: DBApi.playlist.current_item // Only 1 item
        //Layout.margins: 4
        ColumnLayout {
            spacing: 0
            //anchors.margins: 5
            anchors.fill: parent
            anchors.leftMargin: 4
            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //padding: 2
                //Layout.topMargin: 4
                text: ItemTitle
                //Layout.margins: 2
                font.pixelSize: 14
                font.bold: true
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //padding: 2
                //Layout.bottomMargin: 8
                text: ItemArtist
                font.pixelSize: 12
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                Layout.fillWidth: true

            }
        }
        Component.onCompleted: {
            console.log("COLOR:::", Material.foreground)
        }
    }
}

/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
