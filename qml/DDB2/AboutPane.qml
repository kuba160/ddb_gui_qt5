import QtQuick 2.15
//import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0

Page {
    id: mainPane

    //anchors.fill: parent
    header: ToolBar {
        //Material.foreground: "white"
        Material.background: Material.accent
        anchors.margins: 5
        //Material.accent: "white"
        ToolButton {
            anchors.left: parent.left
            anchors.margins: 5
            text: "Back"
            icon.name: "go-previous"

            onClicked: {
                if (stack.depth > 1) {
                    stack.pop()
                }
            }
        }

        Label {
            anchors.centerIn: parent
            id: deadbeef_label
            text: "About DeaDBeeF"
            font.pixelSize: 20
            elide: Label.ElideRight
        }
    }

    ColumnLayout {
        anchors.fill: parent
        TabBar {
            currentIndex: swipe.currentIndex
            onCurrentIndexChanged: {
                swipe.currentIndex = currentIndex
            }
            Layout.fillWidth: true
            TabButton {
                text: "DeaDBeeF"
            }
            TabButton {
                text: "Q User Interface"
            }
        }

        SwipeView {
            id: swipe
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true

            ScrollView {
                id: mainview
                clip: true
                enabled: true
                //anchors.fill: parent
                ColumnLayout {
                    TextEdit {
                        id: edit_text
                        Layout.margins: 16
                        //Layout.fillWidth: true
                        color: deadbeef_label.color
                        readOnly: true
                        selectByMouse: true
                        text: DBApi.conf.aboutText

                        Shortcut {
                            sequences: [StandardKey.Copy]
                            onActivated: edit_text.copy()
                        }
                    }
                }
                ScrollBar.horizontal.policy: width < implicitWidth ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: height < implicitHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
             }
            Label {
                text: "test"
            }
        }
    }
}


/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
