import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qml"
import DBApi 1.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.12

ApplicationWindow {
    id: windo
    width: 640
    height: 480
    visible: true
    Material.accent: Material.Teal
    Material.theme: Material.Dark

    // Titlebar
    Connections {
        id: conns
        target: playback
        function onCurrentTrackChanged() {
            if (playback.stopped) {
                windo.title = playback.tf_current(conf.get("MainWindow", "titlebar_stopped_tf", "DeaDBeeF"))
            }
            else {
                windo.title = playback.tf_current(conf.get("MainWindow", "titlebar_playing_tf", "DeaDBeeF - %artist% - %title%"))
            }
        }
        Component.onCompleted: {
            onCurrentTrackChanged()
        }
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
                Layout.preferredHeight: 64
                spacing: 10
                /*
                ToolButton {
                    icon.name: mainDrawer.folded ? "arrow-right" : "arrow-left"
                    //text: "Drawer"
                    onClicked: {
                        mainDrawer.folded = !mainDrawer.folded
                    }
                    Layout.preferredWidth: 64
                }*/

                PlaybackButtons {
                    //Layout.margins: 4
                    Component.onCompleted: {
                        instance = 0
                    }
                }
                SeekSlider {
                    Component.onCompleted: {
                        instance = 0
                    }
                }
                VolumeSliderQuick {
                    Component.onCompleted: {
                        instance = 0
                    }
                }
            }
            footer: RowLayout {
                spacing: 5
                Label {
                    id: pos
                    property real curr: 0

                    text: (playback.playing ? "Playing" : playback.paused ? "Paused" : "Stopped") + " " +
                          "Pos: " + curr.toFixed(1) + ", Track Length: " + playback.position_max.toFixed(1) + "s"
                    Timer {
                        interval: 10
                        running: true
                        repeat: true
                        onTriggered: {
                            pos.curr = playback.position_rel
                            //console.log(playback.position)
                        }
                    }
                }
            }
            ColumnLayout {
                anchors.fill: parent
                TabBarQuick {
                    //Layout.fillWidth: true
                    Component.onCompleted: {
                        instance = 0
                    }
                }
                RowLayout {
                    spacing: 2

                    MainDrawer {
                        id: mainDrawer
                        parent: parent
                    }

                    //SplitView {
                     //   orientation: Qt.Vertical

                        //Layout.fillWidth: true
                        PlaylistQuick {
                            //SplitView.fillHeight: true
                            Layout.fillHeight: true
                            Component.onCompleted: {
                                instance = 0
                            }
                        }
                      //  QueueManagerQuick {
                            //Layout.preferredHeight: 64
                      //      Component.onCompleted: {
                      //          instance = 0
                       //     }

                      //  }
                    //}



                    /*
                    TableView {
                        clip: true
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        //anchors.fill: parent
                        model: playlist.current
                        delegate: Rectangle {
                            implicitWidth: lab.implicitWidth + 10
                            implicitHeight: lab.implicitHeight + 5
                            clip: true
                            border.width: 1
                            color: "transparent"
                            Label {
                                id: lab
                                text: ItemDisplay
                                leftInset: 3
                                anchors.verticalCenter: parent.verticalCenter
                                wrapMode: Text.WrapAnywhere
                            }
                        }
                    }
                    */
                    /*
                    SplitView {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.maximumHeight: Math.min(cov.implicitHeight,parent.height/2)
                        implicitHeight: 200
                        CoverArt {
                            id: cov
                            //Layout.maximumWidth: Math.max(parent.height/2,parent.width/2)
                            //Layout.preferredHeight: Math.min(parent.height/2,parent.width/2)
                            Layout.preferredWidth: Math.min(implicitWidth, implicitHeight)
                            Component.onCompleted: {
                                instance = 0
                            }
                        }
                        InfoQuick {
                            //Layout.maximumWidth: Math.max(parent.height/2,parent.width/2)
                            //Layout.preferredHeight: Math.min(parent.height/2,parent.width/2)
                            Component.onCompleted: {
                                instance = 0
                            }
                        }
                    }
                    */
                }
            }
        }
    }

}
