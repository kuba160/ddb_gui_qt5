import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

Page {
    property color headerBackgroundColor: Material.accent
    property var page_menu: undefined

    id: page

    header: ToolBar {
        Material.background: page.headerBackgroundColor
        anchors.margins: 5

            width: parent.width
            spacing: 10
            ToolButton {
                id: back
                anchors.left: parent.left
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
                text: page.title
                font.pixelSize: 18
                elide: Label.ElideRight
            }

            ToolButton {
                id: prefs
                icon.name: "overflow-menu"
                anchors.right: parent.right
                visible: page_menu === undefined ? 0 : 1

                onClicked: {
                    page_menu.popup()
                }
            }
    }

}
