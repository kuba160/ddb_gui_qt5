import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

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
        model: playlist.current_item // Only 1 item
        ColumnLayout {
            anchors.fill: parent
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //padding: 2
                text: ItemTitle
                font.pixelSize: 18
                font.bold: true
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            Label {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                //padding: 2
                text: ItemArtist
                font.pixelSize: 18
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                Layout.fillWidth: true

            }
        }
    }
}

/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
