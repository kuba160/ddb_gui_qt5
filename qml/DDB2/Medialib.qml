import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0
import "."

DDB2Page {
    id: page
    title: "Media Library"
    page_menu: Menu {
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
                DDB2Globals.stack.push("MedialibPresetPage.qml")
            }
        }
    }

    Component.onCompleted: {
        //search_line.forceActiveFocus();
    }


    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        RowLayout {
            Layout.margins: 4
            Layout.fillWidth: true
            ComboBox {
                id: preset_chooser
                model: DBApi.playlist.medialib.presets
                currentIndex: DBApi.playlist.medialib.preset_idx
                onCurrentIndexChanged: {
                    DBApi.playlist.medialib.preset_idx = currentIndex
                }
            }
            TextField {
                id: search_line
                Layout.fillWidth: true
                placeholderText: "Search"
                text: DBApi.playlist.medialib.filter
                onTextEdited: {
                    DBApi.playlist.medialib.filter = text
                }
            }
        }

        TreeView {
            id: tree_view
            model: DBApi.playlist.medialib
            clip:true
            Layout.fillHeight: true
            Layout.fillWidth: true
            flickableDirection: Flickable.VerticalFlick
            reuseItems: false
//            pointerNavigationEnabled: false

            delegate: TreeViewDelegate {
                id: treeDelegate
                implicitWidth: tree_view.width
                icon.name: (treeDelegate.isTreeNode && treeDelegate.hasChildren) ?
                                model.IsExpanded ? "arrow-down" : "arrow-right" :
                                ""
                expanded: model.IsExpanded



                Component.onCompleted: {
                    if(model.IsExpanded)
                        treeView.expand(row)
                }


                onClicked: {
                    model.IsExpanded = !model.IsExpanded
                    if (treeView.isExpanded(row))
                        treeView.collapse(row)
                    else
                        treeView.expand(row)
                }

//                ButtonGroup {
//                    id: childGroup
//                    exclusive: false
//                }
                contentItem: RowLayout {
                    spacing: 2
                    Label {
                        text: model.display
                        Layout.fillWidth: true
                        wrapMode: Text.WrapAnywhere
                        maximumLineCount: 1
                        elide: Text.ElideRight
                        font.bold: model.IsSelected ? true : false
                        //color : model.IsSelected ? Material.accent : undefined
                    }


                    CheckBox {
                        id: checkbox
                        Layout.fillHeight: true
                        height: parent.height
                        checked: model.IsSelected ? Qt.Checked : Qt.Unchecked
    //                    checkState: model.IsPartiallySelected ? Qt.PartiallyChecked :
    //                                model.IsSelected ? Qt.Checked :
    //                                                   Qt.Unchecked

                        onClicked: {
                            model.IsSelected = checked;

                        }
                        // avoid scrollbar interference
                        Layout.rightMargin: 8
                    }
                }
            }

            ScrollBar.vertical: ScrollBar {}
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
            spacing: 0
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
