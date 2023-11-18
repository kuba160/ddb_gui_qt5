import QtQuick
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
                text: "Media Library Presets"
                font.pixelSize: 18
                elide: Label.ElideRight
            }


            ToolButton {
                id: prefs
                icon.name: "overflow-menu"
                anchors.right: parent.right

                onClicked: {
                    overflow_menu.popup()

                }

                Menu {
                    id: overflow_menu
                    MenuItem {
                        text: "Folders"
                        onTriggered: {
                            DBApi.playlist.medialib_folders = ["/NAS/Media/Muzyka/FLAC/OWN/Falco/"]
                        }
                    }
                    MenuItem {
                        text: "Presets"
                        onTriggered: {

                        }
                    }
                }
            }


    }

    ColumnLayout {
        anchors.fill: parent

        TreeView {
            id: tree_view
            focus: false
            Layout.fillHeight: true
            Layout.fillWidth: true
            //anchors.fill: parent
            model: DBApi.playlist.medialib.queries
            flickableDirection: Flickable.VerticalFlick
            reuseItems: false
            pointerNavigationEnabled: false

            delegate: TreeViewDelegate {
                id: treeDelegate
                icon.name: (treeDelegate.isTreeNode && treeDelegate.hasChildren) ?
                                treeDelegate.expanded ? "arrow-down" : "arrow-right" :
                                ""
                text: model.display
                font.bold: false
                implicitWidth: tree_view.width


                onClicked: {
                    if (treeView.isExpanded(row))
                        treeView.collapse(row)
                    else
                        treeView.expand(row)
                }

            }

            ScrollBar.vertical: ScrollBar {}
            Component.onCompleted: {
                tree_view.expandRecursively(-1,1)
            }
        }
//        ItemDelegate {
//            anchors.centerIn: parent
//            text: "Use search field to find tracks."
//            visible: search_line.length <= 0
//            opacity: visible ? 1 : 0
//            enabled: false
//        }
    }
    footer: ToolBar {
        enabled: false
        Material.background: Material.color(Material.Grey, Material.Shade800)
        width: parent.width
        RowLayout {
//            Label {
//                text: "Queue"
//            }
            Layout.alignment: Qt.AlignHCenter
            anchors.fill: parent

            ToolButton {
                icon.name: "go-first"
                text: "Play next"
                Layout.fillWidth: true
            }
            ToolButton {
                icon.name: "list-add"
                text: "Play later"
                Layout.fillWidth: true
            }
//            Label {
//                text: "Playlist"
//            }
            ToolButton {
                icon.name: "media-playlist-append"
                text: "Add to playlist"
                Layout.fillWidth: true
            }
        }
    }
}
