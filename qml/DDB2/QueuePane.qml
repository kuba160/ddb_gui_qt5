import QtQuick
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

            width: parent.width
            spacing: 10
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
                text: "Queue"
                font.pixelSize: 18
                elide: Label.ElideRight
            }

//            Label {
//                text: "Search:"
//            }

//            TextField {
//                id: search_line
//                Layout.margins: 2
//                Layout.alignment: Qt.AlignBottom
//                Layout.rightMargin: 32
//                Layout.fillWidth: true
//                Layout.maximumHeight: parent.implicitHeight
//                placeholderText: "Search music"
//                text: DBApi.playlist.current_search.item_filter
//                //height: parent.height

//                onTextChanged: {
//                    DBApi.playlist.current_search.item_filter = text
//                }
//            }

            ToolButton {
                id: prefs
                icon.name: "overflow-menu"
                anchors.right: parent.right

                // clear queue menu
            }


    }

    Item {
        anchors.fill: parent
        PlayItemView {
            id: piv
            anchors.fill: parent
            model: DBApi.playlist.queue
            delegate: FocusScope {
                width: parent.width; height: childrenRect.height
                x:childrenRect.x; y: childrenRect.y
                DDB2PlayItemViewDelegate {}}

            ItemDelegate {
                anchors.centerIn: parent
                text: "Queue is empty"
                visible: piv.count === 0
                opacity: visible ? 1 : 0
                enabled: false
            }
        }
    }
}
