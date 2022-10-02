import QtQuick 2.15
import QtQuick.Controls 2.12

import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.12

DBWidget {
    friendlyName: qsTr("Playlist")
    internalName: "playlist"
    widgetStyle: "Qt Quick"
    widgetType: "main"

    widget: ColumnLayout {
        anchors.fill: parent
        PlaylistView {
            id : playlistView
            model: playlist.current
        }
        ToolBar {
            visible: false
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBottom
            Material.background: Material.accent
            RowLayout {
                ToolButton {
                    text: "Unselect"
                }

                Label {
                    text: "Currently selected " + playlistView.checkedItems + " items."
                }

                ToolButton {
                    text: "+"
                }
            }
        }
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 400
    Layout.preferredHeight: 400
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
