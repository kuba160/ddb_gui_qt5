import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import DBApi 1.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0
import "."

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

    Connections {
        target: DBApi.actions.getAction("q_about")
        function onActionApplied() {
            stack.push(about_pane)
        }
    }

    function about_push(pane, extra_pane) {
        return function() {
            stack.push(about_pane, {defaultPane: pane, extraPane: extra_pane})
        }
    }
    Component.onCompleted: {
        DBApi.actions.getAction("q_about").actionApplied.connect(about_push(0,0))
        DBApi.actions.getAction("q_changelog").actionApplied.connect(about_push(0,1))
        DBApi.actions.getAction("q_gplv2").actionApplied.connect(about_push(0,2))
        DBApi.actions.getAction("q_lgplv21").actionApplied.connect(about_push(0,3))
        DBApi.actions.getAction("q_translators").actionApplied.connect(about_push(2))
        DBApi.actions.getAction("q_quit").actionApplied.connect(close)

        DDB2Globals.window = this
    }

    Component {
        id: about_pane
         AboutPane {}
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
             Component.onCompleted: {
                 console.log("settings stack", stack)
                 DDB2Globals.stack = stack
             }
    }

    Component {
        id: mainView
        Page {
            header: RowLayout {
                Layout.margins: 20
                Layout.preferredHeight: 128
                height: implicitHeight
                spacing: 2
                width: parent.width

                ToolButton {
                    icon.name: "application-menu"
                    //text: "Drawer"
                    onClicked: {
                        drawer.open()
                    }
                    //Layout.preferredWidth: 64
                }


                InfoQuick {
                    Layout.fillWidth: true
                }

//                ToolButton {
//                    icon.name: "media-playlist-append"
//                    //text: "Drawer"
//                    //Layout.preferredWidth: 64
//                    onClicked: {
//                        DBApi.actions.execAction("q_new_playlist")
//                    }
//                }


                ToolButton {
                    icon.name: "insert-more-mark"
                    //text: "Drawer"
                    //Layout.preferredWidth: 64
                    Component {
                        id: queue_pane
                         QueuePane {}
                    }
                    onClicked: {
                        stack.push(queue_pane)
                    }
                    Label {
                        anchors.margins: 8
                        anchors.bottomMargin: 6
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        text: DBApi.playlist.queue.length >= 100 ? "XD" : DBApi.playlist.queue.length
                        visible: DBApi.playlist.queue.length
                        font.pixelSize: 12
                        font.bold: true
                        style: Text.Outline
                        styleColor: Material.backgroundColor //Material.color(Material.Grey, Material.Shade800)
                        z: 5

                    }
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

                    Component {
                        id: menu_general
                        ActionMenuMain {
                            prototype_id: 3
                        }
                    }

                    onClicked: {
                        let menu = menu_general.createObject(this)
                        menu.open()
                    }
                    //text: "Drawer"
                    //Layout.preferredWidth: 64
                }
            }

            ColumnLayout {
                anchors.fill: parent
                SeekSlider {}

                PlayItemView {
                    id: piv
                    model: DBApi.playlist.current
                    delegate: FocusScope {
                        width: parent.width; height: childrenRect.height
                        x:childrenRect.x; y: childrenRect.y
                        DDB2PlayItemViewDelegate {}}

                    ItemDelegate {
                        anchors.centerIn: parent
                        text: "Playlist is empty"
                        visible: piv.count == 0
                        opacity: visible ? 1 : 0
                        enabled: false
                    }
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

                        Component {
                            id: cover_popup
                            Popup {
                                id: popup
                                anchors.centerIn: parent
                                width: Math.floor(Math.min(parent.width,parent.height)*0.8)
                                height: Math.floor(Math.min(parent.width,parent.height)*0.8)
                                //modal: true
                                CoverArt {
                                        width: parent.width
                                        height: parent.height
                                        anchors.centerIn: parent
                                }
                                onClosed: {
                                    destroy()
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                var popup = cover_popup.createObject(windo)
                                popup.open()
                            }

                        }
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
                        spacing: 4


                        RepeatShuffleButton {
                            button_action: Action {
                                text: "Previous"
                                icon.name: "media-skip-backward"
                                onTriggered: {
                                    DBApi.playback.prev()
                                }
                            }
                            anchors.verticalCenter: parent.verticalCenter
                            icon.width: 24
                            icon.height: 24
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
                            onPressAndHold: {
                                DBApi.playback.stop()
                            }

                            anchors.verticalCenter: parent.verticalCenter
                            icon.width: 32
                            icon.height: 32
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
                            icon.width: 24
                            icon.height: 24
                        }
                    }
                    VolumeButton {
                        anchors.right: shuffle_button.left
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    RepeatShuffleButton {
                        id: shuffle_button
                        button_action: Action {
                            id: action_shuffle
                            checkable: true
                            property var shuffle_str: ["Shuffle Off", "Shuffle Tracks", "Shuffle Albums", "Shuffle Random"]
                            property var shuffle_ico: ["media-playlist-normal", "media-random-tracks-amarok", "media-random-albums-amarok", "media-playlist-shuffle"]
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
