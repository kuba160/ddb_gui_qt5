import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import DBApi 1.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

ApplicationWindow {
    id: windo
    width: 640
    height: 480
    visible: true
    Material.accent: Material.Teal
    Material.theme: Material.Dark
    //Material.theme: Material.Light

    // Titlebar
    Connections {
        id: conns
        target: DBApi.playback
        function onCurrentTrackChanged() {
            if (DBApi.playback.stopped) {
                windo.title = DBApi.playback.tf_current(DBApi.conf.get("MainWindow", "titlebar_stopped_tf", "DeaDBeeF"))
            }
            else {
                windo.title = DBApi.playback.tf_current(DBApi.conf.get("MainWindow", "titlebar_playing_tf", "DeaDBeeF - %artist% - %title%"))
            }
        }
        Component.onCompleted: {
            onCurrentTrackChanged()
        }
    }

    MainDrawer {
        id: drawer
        stack: stack
    }

//    menuBar: MenuBar {
//        Repeater {
//            model: actions.actions

//            Menu {
//                text: display
//            }
//        }
//    }


/*
    Drawer {
        id: playlistBrowserDrawer
        edge: Qt.RightEdge
        width: Math.max(256, 0.33 * parent.width)
        //dragMargin: Qt.styleHints.startDragDistance
        height: parent.height

        ColumnLayout {
            anchors.fill: parent
            Label {
                Layout.leftMargin: 8
                text: "Playlists:"
            }

            PlaylistBrowserQuick {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Component.onCompleted:  {
                    instance = 998
                }
            }
        }
    }*/

    StackView {
             id: stack
             anchors.fill: parent
             initialItem: mainView
    }

    Component {
        id: mainView
        Page {
            header: RowLayout {
                Layout.margins: 20
                Layout.preferredHeight: 128
                spacing: 2

                ToolButton {
                    icon.name: "application-menu"
                    //text: "Drawer"
                    onClicked: {
                        drawer.open()
                    }
                    //Layout.preferredWidth: 64
                }


                InfoQuick {}

                ToolButton {
                    icon.name: "media-playlist-append"
                    //text: "Drawer"
                    //Layout.preferredWidth: 64
                }
                ToolButton {
                    icon.name: "search"
                    //text: "Drawer"
                    //Layout.preferredWidth: 64
                    Component {
                        id: search_pane
                         SearchPane {}
                    }
                    onClicked: {
                        stack.push(search_pane)
                    }
                }
                ToolButton {
                    icon.name: "overflow-menu"
                    //text: "Drawer"
                    //Layout.preferredWidth: 64
                }
            }

            ColumnLayout {
                anchors.fill: parent
                SeekSlider {}

                PlayItemView {
                    model: DBApi.playlist.current
                    delegate: DDB2PlayItemViewDelegate {}
                }

                Rectangle {
                    SystemPalette {id: pal}
                    color: pal.alternateBase
                    Layout.fillWidth: true
                    height: 48
                    CoverArt {
                        id: cover

                        anchors.margins: 4
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        width: height
                    }
                    RepeatShuffleButton {
                        button_action: Action {
                            id: action_repeat
                            property var repeat_str: ["Repeat All", "Repeat Off", "Repeat Single"]
                            property var repeat_ico: ["media-repeat-all", "media-repeat-none", "media-repeat-single"]
                            text: repeat_str[DBApi.playback.repeat]
                            icon.name: repeat_ico[DBApi.playback.repeat]
                            onTriggered: {
                                DBApi.playback.repeat = (DBApi.playback.repeat+1) % repeat_str.length
                            }
                        }
                        action: action_repeat
                        anchors.left: cover.right
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        RepeatShuffleButton {
                            button_action: Action {
                                text: "Previous"
                                icon.name: "media-skip-backward"
                                onTriggered: {
                                    DBApi.playback.prev()
                                }
                            }
                            anchors.verticalCenter: parent.verticalCenter
                            icon.width: 32
                            icon.height: 32
                            //text: "Drawer"
                            //Layout.preferredWidth: 64
                        }
                        RepeatShuffleButton {
                            button_action: Action {
                                text: DBApi.playback.playing ? "Pause" : "Play"
                                icon.name: DBApi.playback.playing ? "media-playback-pause" : "media-playback-start"
                                onTriggered: {
                                    DBApi.playback.pause()
                                }
                            }
                            anchors.verticalCenter: parent.verticalCenter
                            icon.width: 42
                            icon.height: 42
                            flat: true

                            //Layout.preferredWidth: 64
                        }
                        RepeatShuffleButton {
                            button_action: Action {
                                text: "Forward"
                                icon.name: "media-skip-forward"
                                onTriggered: {
                                    DBApi.playback.next()
                                }
                            }
                            anchors.verticalCenter: parent.verticalCenter
                            icon.width: 32
                            icon.height: 32
                        }
                    }
                    RepeatShuffleButton {
                        button_action: Action {
                            id: action_shuffle
                            checkable: true
                            property var shuffle_str: ["Shuffle Off", "Shuffle Tracks", "Shuffle Random", "Shuffle Albums"]
                            property var shuffle_ico: ["media-playlist-normal", "media-random-tracks-amarok", "media-playlist-shuffle", "media-random-albums-amarok"]
                            text: shuffle_str[DBApi.playback.shuffle]
                            icon.name: shuffle_ico[DBApi.playback.shuffle]
                            onTriggered: {
                                DBApi.playback.shuffle = (DBApi.playback.shuffle+1) % shuffle_str.length
                            }
                        }
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        icon.width: 32
                        icon.height: 32
                    }
                }
            }
        }
    }
}
