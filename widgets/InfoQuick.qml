import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
    id: main
    readonly property string friendlyName: qsTr("Info")
    readonly property string internalName: "infoQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "main"
    property int instance

    Loader {
        id: loader
        sourceComponent: api === null ? undefined : info
        // size determined by rootItem (corresponding to QWidget size)
        width: parent.width
        height: parent.height
        asynchronous: false
    }
    Component {
        id: info
        Repeater {
            model: api.current_playing_model
            Item {
                anchors.fill: parent
                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    Label {
                        padding: 2
                        text: ItemTitle
                        font.pixelSize: 22
                        font.bold: true
                    }
                    Label {
                        padding: 2
                        text: ItemArtist
                        font.pixelSize: 18
                    }
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
