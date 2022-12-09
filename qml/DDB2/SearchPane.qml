import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

Page {
    id: mainPane
    //anchors.fill: parent
    header: ToolBar {
        //Material.foreground: "white"
        Material.background: Material.accent
        anchors.margins: 5
        //Material.accent: "white"
        RowLayout {
            width: parent.width
            spacing: 10
            ToolButton {
                Layout.alignment: Qt.AlignLeft
                Layout.margins: 5
                //text: "Back"
                icon.name: "go-previous"
                onClicked: {
                    if (stack.depth > 1) {
                        stack.pop()
                    }
                }
            }

            TextField {
                id: search_line
                Layout.margins: 2
                Layout.alignment: Qt.AlignBottom
                Layout.rightMargin: 32
                Layout.fillWidth: true
                placeholderText: "Search music"
                text: DBApi.playlist.current_search.item_filter

                onTextChanged: {
                    DBApi.playlist.current_search.item_filter = text
                }
            }
        }

    }

    PlayItemView {
        anchors.fill: parent
        model: DBApi.playlist.current_search
        delegate: DDB2PlayItemViewDelegate {}
    }
}
