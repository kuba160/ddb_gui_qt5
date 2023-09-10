import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

Page {
    id: mainPane
    //anchors.fill: parent
    Component.onCompleted: {
        search_line.forceActiveFocus();
    }


    header: ToolBar {
        //Material.foreground: "white"
        Material.background: Material.accent
        anchors.margins: 5
        //Material.accent: "white"

        width: parent.width
        //spacing: 10
        ToolButton {
            id: back
            anchors.left: parent.left

            //text: "Back"
            icon.name: "go-previous"

            function goBack() {
                if (stack.depth > 1) {
                    stack.pop()
                }
            }

            onClicked: {
                goBack()
            }


            Shortcut {
                sequence: StandardKey.Cancel
                onActivated: {
                    back.goBack()
                }
            }
        }
        Label {
            anchors.centerIn: parent
            id: deadbeef_label
            text: "Search"
            font.pixelSize: 18
            elide: Label.ElideRight
        }
    }


    ColumnLayout {
        anchors.fill: parent

        TextField {
            id: search_line
            //Layout.margins: 2
            padding: 8
            Layout.margins: 8
            Layout.fillWidth: true
            Layout.maximumHeight: parent.implicitHeight
            placeholderText: "Search music"
            text: DBApi.playlist.current_search.item_filter
            //height: parent.height

            onTextChanged: {
                DBApi.playlist.current_search.item_filter = text
            }
        }

        Item {
            //anchors.fill: parent
            Layout.fillHeight: true
            Layout.fillWidth: true
            PlayItemView {
                focus: false
                anchors.fill: parent
                model: DBApi.playlist.current_search
                delegate: DDB2PlayItemViewDelegate {}
            }
            ItemDelegate {
                anchors.centerIn: parent
                text: "Use search field to find tracks."
                visible: search_line.length <= 0
                opacity: visible ? 1 : 0
                enabled: false
            }
        }
    }
}
