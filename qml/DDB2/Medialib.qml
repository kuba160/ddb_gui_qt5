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
                text: "Media Library"
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

                onClicked: {
                    DBApi.playlist.medialib_folders = ["/NAS/Media/Muzyka/FLAC/OWN/Falco/"]
                }
            }


    }

    Item {
        anchors.fill: parent
        TreeView {
            focus: false
            anchors.fill: parent
            model: DBApi.playlist.medialib
            flickableDirection: Flickable.VerticalFlick
            //delegate: DDB2PlayItemViewDelegate {}

            Transition {
                id: treeTransitionAnimation
                NumberAnimation { properties: "y"; duration: 300 }
            }
            delegate: ItemDelegate {
                id: treeDelegate
                icon.name: (treeDelegate.isTreeNode && treeDelegate.hasChildren) ?
                                treeDelegate.expanded ? "arrow-down" : "arrow-right" :
                                ""
                text: model.display
                font.bold: false
                implicitWidth: parent.width
                leftPadding: treeDelegate.depth * 32 + 4
                rightPadding: checkbox.width + 24

//                ButtonGroup {
//                    id: childGroup
//                    exclusive: false
//                }

                CheckBox {
                    id: checkbox
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    height: parent.height
                    checked: model.IsSelected ? Qt.Checked : Qt.Unchecked
//                    checkState: model.IsPartiallySelected ? Qt.PartiallyChecked :
//                                model.IsSelected ? Qt.Checked :
//                                                   Qt.Unchecked

                     //ButtonGroup.group: parent.childGroup
                    onClicked: {
                        model.IsSelected = checked
                    }
                }

                //implicitWidth: padding + label.x + label.implicitWidth + padding
                //implicitHeight: label.implicitHeight * 1.5

//                readonly property real indent: 20
//                readonly property real padding: 5

                // Assigned to by TreeView:
                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property int hasChildren
                required property int depth

                onClicked: {
                    if (!(treeDelegate.isTreeNode && treeDelegate.hasChildren)) {
                        checkbox.checked = !checkbox.checked
                    }
                    else {
                        if (treeDelegate.expanded) {
                            treeView.collapseRecursively(row)
                        }
                        else {
                            treeView.toggleExpanded(row)
                        }
                    }
                }


//                TapHandler {
//                    onTapped: {

//                    }
//                }
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
