import QtQuick 2.15
import QtQuick.Controls 2.12

import Qt.labs.qmlmodels 1.0

Item {
    readonly property string friendlyName: qsTr("Playlist")
    readonly property string internalName: "playlistQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "main"
    property int instance: -1

    Loader {
        id: loader
        sourceComponent: instance >= 0 ? playlist : undefined
        // size determined by rootItem (corresponding to QWidget size)
        width: Math.max(parent.width, 200)
        height: parent.height
    }
    Component {
        id: playlist

        Item {
            anchors.fill: parent

            /*
            ComboBox {
                id: toolbar
                model: api.playlists
                textRole: "playlistName"
                visible: true
                currentIndex: api.current_playlist
                onActivated: {
                    api.current_playlist = index
                }
                Component.onCompleted: {
                    ComboBox.popup.open()
                }
            }*/

            ListView {
                id: listview
                anchors.fill: parent
                anchors.margins: 2
                //anchors.top: toolbar.bottom
                //implicitHeight: parent.height - toolbar.height
                //anchors.top: header_repeater
                //height: parent.height - header_row
                //columnSpacing: 1
                spacing: 1
                //clip: true

                model: api.current_playlist_model

                delegate: SwipeDelegate {
                    id: it_delegate

                    implicitHeight: Math.max(checkbox.height, image_rect.height)
                    width: parent ? parent.width : undefined
                    //height:

                    highlighted: checkbox.checked///*ItemSelected ? ItemSelected : false //*/ItemPlayingState ? ItemPlayingState : false
                    onDoubleClicked: {
                        api.playTrackByIndex(index)
                    }
                    onClicked: {
                        checkbox.checked = true
                    }

                    ListView.onRemove: SequentialAnimation {
                        PropertyAction {
                            target: it_delegate
                            property: "ListView.delayRemove"
                            value: true
                        }
                        NumberAnimation {
                            target: it_delegate
                            property: "height"
                            to: 0
                            easing.type: Easing.InOutQuad
                        }
                        PropertyAction {
                            target: it_delegate
                            property: "ListView.delayRemove"
                            value: false
                        }
                    }
                    swipe.left: Label {
                        id: queueLabel
                        text: qsTr("Add To Playback Queue")
                        color: "white"
                        verticalAlignment: Label.AlignVCenter
                        padding: 12
                        height: parent.height
                        anchors.left: parent.left



                        background: Rectangle {
                            color: queueLabel.SwipeDelegate.pressed ? Qt.darker("tomato", 1.1) : "tomato"
                        }
                    }
                    swipe.right: Label {
                        id: deleteLabel
                        text: qsTr("Delete")
                        color: "white"
                        verticalAlignment: Label.AlignVCenter
                        padding: 12
                        height: parent.height
                        anchors.right: parent.right

                        //SwipeDelegate.onClicked: listView.model.remove(index)

                        background: Rectangle {
                            color: deleteLabel.SwipeDelegate.pressed ? Qt.darker("tomato", 1.1) : "tomato"
                        }
                    }
                    Item {
                        id: row_delegate
                        anchors.fill: parent
                        //spacing: 4
                        CheckBox {
                            id: checkbox
                            checked: ItemSelected
                        }

                        Rectangle {
                            id: image_rect
                            anchors.left: checkbox.right
                            anchors.verticalCenter: parent.verticalCenter
                            color: palette.window
                            height: 32
                            width: height

                            border.color: "dimgrey"
                            border.width: 1
                            SystemPalette { id: palette; colorGroup: SystemPalette.Active }
                            Item {
                                anchors.fill: parent
                                Label {
                                    id: tracknum
                                    anchors.fill: parent
                                    //anchors.verticalCenter: image_rect2
                                    //anchors.horizontalCenter: image_rect2
                                    text: ItemTrackNum
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter

                                    opacity: ItemPlayingState ? 0 : 1
                                    Behavior on opacity { SmoothedAnimation { velocity: 2 } }
                                }
                                Image {
                                    width: 16
                                    height: 16
                                    source: "/root/images/play_16.png"
                                    anchors.centerIn: parent
                                    opacity: ItemPlayingState  === 1 ? 1 : 0
                                    Behavior on opacity { SmoothedAnimation { velocity: 2 } }
                                }
                                Image {
                                    width: 16
                                    height: 16
                                    source: "/root/images/pause_16.png"
                                    anchors.centerIn: parent
                                    opacity: ItemPlayingState  === 2 ? 1 : 0
                                    Behavior on opacity { SmoothedAnimation { velocity: 2 } }
                                }
                            }
                        }


                        Column {
                            anchors.left: image_rect.right
                            anchors.right: queue_label.left
                            anchors.verticalCenter: parent.verticalCenter
                            leftPadding: 4
                            bottomPadding: 4
                            Label {
                                id: text1
                                text: ItemTitle

                                width: parent.width
                                wrapMode: Text.WrapAnywhere
                                maximumLineCount: 1
                                elide: Text.ElideRight


                            }
                            Label {
                                id: text2
                                text: ItemArtist
                                font.pixelSize: 10
                                width: parent.width
                                wrapMode: Text.WrapAnywhere
                                maximumLineCount: 1
                                elide: Text.ElideRight
                            }
                        }
                        ToolButton {
                            id: toolbtn
                            text: "⋮"
                            anchors.right: parent.right
                            anchors.rightMargin: scrollbar.width
                            //anchors.verticalCenter: parent.verticalCenter
                            flat: true
                        }
                        Label {
                            id: queue_label
                            text: ItemPlaying ? ItemPlaying : ""
                            anchors.right: toolbtn.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 2
                        }
                    }


                }
                ScrollBar.vertical: ScrollBar {
                    id: scrollbar
                    active: true
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
