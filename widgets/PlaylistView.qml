import QtQuick 2.15
import QtQuick.Controls 2.12

import Qt.labs.qmlmodels 1.0


ListView {
    readonly property string widgetType: "ignore"
    id: listview
    anchors.fill: parent
    anchors.margins: 2
    spacing: 1

    ScrollBar.vertical: ScrollBar {
        id: scrollbar
        active: true
    }

    delegate: ItemDelegate {
        id: it_delegate
        implicitHeight: Math.max(checkbox.height, image_rect.height)
        width: parent ? parent.width : undefined

        highlighted: checkbox.checked
        onDoubleClicked: {
            api.playTrackByIndex(index)
        }

        Item {
            id: row_delegate
            anchors.fill: parent

            // Selection checkbox
            CheckBox {
                id: checkbox
                checked: ItemPlayingState ? true : false //ItemSelected
            }

            // Cover / track number / playing indicator
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
            // Title/Artist column
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
            Label {
                id: queue_label
                text: ItemPlaying ? ItemPlaying : ""
                anchors.right: toolbtn.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 2
            }
            ToolButton {
                id: toolbtn
                text: "⋮"
                anchors.right: parent.right
                anchors.rightMargin: scrollbar.width
                width: height
                anchors.verticalCenter: parent.verticalCenter
                //anchors.verticalCenter: parent.verticalCenter
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
