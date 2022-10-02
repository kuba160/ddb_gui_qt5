import QtQuick 2.15
import QtQuick.Controls 2.12

import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.12
// import Qt.labs.platform
import DBApi 1.0

ListView {
    readonly property string widgetType: "ignore"
    property var checkedItems: []
    id: listview
    Layout.fillWidth: true
    Layout.fillHeight: true

    clip: true

    flickDeceleration: 10000
    maximumFlickVelocity: 5000

    reuseItems: true
    spacing: 1

    Keys.onUpPressed: scrollbar.decrease()
    Keys.onDownPressed: scrollbar.increase()

    ScrollBar.vertical: ScrollBar {
        id: scrollbar

        active: true
    }

    section.property: "ItemAlbum"
    section.criteria: ViewSection.FullString
    section.delegate: Label {
        text: section
        font.italic: true
        height: implicitHeight + 4
    }


    /*Menu {
        id: track_menu
        MenuItem {
            text: "Test"
            icon.name: "list-add"
        }
    }*/

    delegate: ItemDelegate {
        id: it_delegate
        width: ListView.view.width - scrollbar.width
        height: row_delegate.implicitHeight
        //width: row_delegate.width

        highlighted: checkbox.checked
        onDoubleClicked: {
            playback.play(index)
        }

        Rectangle {
            id: playing_highlight
            height: 2
            anchors.bottom: parent.bottom
            visible: ItemPlayingState
            implicitWidth: parent.width * playbackProgress

            color: ItemPlayingState === 1 ? Material.accent : "gray"
            property real playbackProgress: 0.0
            Behavior on playbackProgress { SmoothedAnimation { velocity: 300; } }
            Behavior on width { SmoothedAnimation { velocity: 1000; } }

            Timer {
                interval: 100
                running: parent.visible
                repeat: true
                onTriggered: {
                    playing_highlight.playbackProgress = playback.position_abs / 100
                }
                onRunningChanged: {
                    if (!running) {
                        playing_highlight.playbackProgress = 0.0
                    }
                }
            }

        }

        RowLayout {
            Layout.margins: 4
            anchors.fill: parent
            Layout.leftMargin: 8
            id: row_delegate

            // Selection checkbox
            AbstractButton {
                Layout.leftMargin: 8

                Layout.fillHeight: true
                Layout.preferredWidth: checkbox.implicitWidth

                onClicked: {
                    checkbox.checked = !checkbox.checked
                }


                CheckBox {
                    anchors.centerIn: parent
                    id: checkbox
                    onCheckStateChanged: {
                        if (checkState == Qt.Checked) {
                            console.log("BEFORE:", checkedItems)
                            const i = checkedItems.indexOf(index);
                            if (i <= -1)
                                checkedItems.push(index)
                            console.log("AFTER:", checkedItems , i)
                        }
                        else if (checkState == Qt.Unchecked) {
                            console.log("UNCHECK BEFORE:", checkedItems)
                            const i = checkedItems.indexOf(index);
                            if (i > -1) { // only splice array when item is found
                              checkedItems.splice(i, 1); // 2nd parameter means remove one item only
                            }
                            console.log("UNCHECK AFTER:", checkedItems, i)
                        }
                    }

                    //checked: ItemPlayingState ? true : false //ItemSelected
                }

            }



            // Cover / track number / playing indicator
            Rectangle {
                id: rect
                color: "transparent"
                Layout.preferredHeight: 64
                Layout.preferredWidth: Layout.preferredHeight
                Layout.alignment: Qt.AlignLeft

                border.color: "dimgrey"
                border.width: 1
                SystemPalette { id: palette; colorGroup: SystemPalette.Active }

                Component {
                    id: rect_cover
                    Image {
                        id: rect_cover_img
                        mipmap: true
                        smooth: false
                        asynchronous: true
                        //visible: status == Image.Ready
                        opacity: 0
                        z: 10

                        source: ItemAlbumArtUrl !== undefined ? ItemAlbumArtUrl : undefined
                        width: row_delegate.implicitHeight
                        height: row_delegate.implicitHeight
                        Behavior on opacity {
                            NumberAnimation {
                                id: cover_ani
                                target: rect_cover_img
                                property: "opacity"
                                duration: 100
                                easing.type: Easing.InOutQuad
                            }
                        }

                        onStatusChanged: {
                            rect_cover_img.opacity = 1
                            if (status == Image.Ready) {
                                rect_cover_img.opacity = 1
                            }
                            else {
                                rect_cover_img.opacity = 0
                                cover_ani.complete()
                            }
                        }
                    }
                }
                Loader {
                    z: 10
                    sourceComponent: ItemAlbumArtUrl !== undefined ? rect_cover : rect_status
                }

                Component {
                    id: rect_status
                    Image {
                        id: rect_item
                        width: row_delegate.implicitHeight
                        height: row_delegate.implicitHeight
                        visible: true //ItemAlbumArtUrl == undefined
                        Label {
                            id: rect_tracknum
                            anchors.centerIn: parent
                            text: ItemTrackNum
                            //color: palette.windowText
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            z: 5

                            opacity: ItemPlayingState ? 0 : 1
                            Behavior on opacity { SmoothedAnimation { velocity: 3 } }
                        }
                        Image {
                            id: rect_play
                            width: 16
                            height: 16
                            anchors.centerIn: parent
                            source: "/images/play_16.png"
                            opacity: ItemPlayingState  === 1 ? 1 : 0
                            Behavior on opacity { SmoothedAnimation { velocity: 3 } }
                        }
                        Image {
                            id: rect_pause
                            width: 16
                            height: 16
                            anchors.centerIn: parent
                            source: "/images/pause_16.png"
                            opacity: ItemPlayingState  === 2 ? 1 : 0
                            Behavior on opacity { SmoothedAnimation { velocity: 3 } }
                        }
                    }
                }





            }
            // Title/Artist column
            ColumnLayout {
                Layout.alignment: Qt.AlignLeft
                //leftPadding: 4
                //bottomPadding: 4
                Label {
                    id: text1
                    text: ItemTitle
                    //width: parent.width
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    //Layout.maximumWidth: implicitWidth
                    clip: true
                }
                Label {
                    id: text2
                    text: ItemArtist
                    font.pixelSize: 10
                    //width: parent.width
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    //Layout.maximumWidth: implicitWidth
                    clip: true
                }
            }
/*
            Label {
                clip: true
                text: ItemAlbum
                //Layout.fillWidth: true
                visible: parent.width < parent.implicitWidth ? 0 : 1
                //Layout.minimumWidth: implicitWidth
            }
*/
            Label {
                id: queue_label
                text: ItemPlaying ? ItemPlaying : ""
                Layout.alignment: Qt.AlignRight
                font.pixelSize: 16
                //anchors.rightMargin: 2

                //opacity: ItemPlaying ? 1 : 0
                property bool inQueue: ItemPlaying ? 1 : 0

                onTextChanged: {
                    visible_animation.start();
                }
                SequentialAnimation {
                    id: visible_animation
                    ScaleAnimator {
                        target: queue_label
                        //from: 0;
                        to: 1.25;
                        duration: 1000
                    }
                    ScaleAnimator {
                        target: queue_label
                        from: 1.25;
                        to: 1;
                        duration: 1000
                    }
                }

                //Behavior on opacity { SmoothedAnimation { velocity: 2 } }
            }
            ToolButton {
                id: button_queue
                text: ""
                icon.name: "list-add"
                onClicked: {
                    playlist.queueAppend(ItemMime)
                    //playback.queuePush(ItemPtr)
                }
                display: AbstractButton.IconOnly

                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 0
                Layout.preferredWidth: height
                flat: true
            }
            ToolButton {
                id: toolbtn
                text: "⋮"
                Layout.alignment: Qt.AlignRight
                anchors.rightMargin: scrollbar.width
                Layout.preferredWidth: height
                //anchors.verticalCenter: parent.verticalCenter
                //anchors.verticalCenter: parent.verticalCenter

                onClicked: {
                    //track_menu.popup()
                }

                flat: true
            }
        }
    }
}

/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
