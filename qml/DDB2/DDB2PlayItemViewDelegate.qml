import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

ItemDelegate {
    readonly property string widgetType: "ignore"
    //property int idx: index
    //required property int index

    id: it_delegate
    width: parent ? parent.width : undefined//- parent.ScrollBar.vertical.width
    //Layout.fillWidth: true
    //width: ListView.view.width - scrollbar.width
    height: row_delegate.implicitHeight
    //width: row_delegate.width

    highlighted: ItemCursor
    onClicked: {
        ItemCursor = true
    }

    onDoubleClicked: {
        console.log("AAAA", index, ItemIndex, parseInt(ItemIndex))
        DBApi.playback.play(parseInt(ItemIndex))
    }

    background: Item {
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: "gray"
            opacity: 0.1
        }
    }

    padding: 5

    Component {
        id: menu_component
        ActionMenu {
            prototype_id: 1
            itemIterator: ItemIterator
        }
    }

    MouseArea {
        acceptedButtons: Qt.RightButton
        anchors.fill: parent

        onClicked: {
            if (mouse.button === Qt.RightButton) { // 'mouse' is a MouseEvent argument passed into the onClicked signal handler
                let menu = menu_component.createObject(this, {x: mouse.x- 128})
                menu.open()
            }
        }
    }

    // in queue indicator:
    Canvas {
        id: line_canvas
        anchors.fill: parent
        contextType: "2d"
        property var queue_list: ItemQueue

        onQueue_listChanged: requestPaint()

        onPaint: {
            var context = getContext("2d");
            context.clearRect(0,0,line_canvas.width, line_canvas.height)
            // mid line
            if (queue_list != undefined) {
                context.strokeStyle = Material.accent //Qt.rgba(1,1,1,.5);
                context.lineWidth = 1
                context.path = undefined
                context.moveTo(2, 2) //line_canvas.height/2)
                context.lineTo(2, line_canvas.height - 2);
                context.stroke()
            }
        }
        z: 2
    }


    /*Rectangle {
        id: playing_highlight
        height: 2
        anchors.bottom: parent.bottom
        visible: ItemPlayingState
        implicitWidth: parent.width * playbackProgress
        z: 10

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
    }*/

    property bool isPlaying: ItemPlayingState === 1 || ItemPlayingState === 2

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 4
        GridLayout {
            Layout.fillWidth: true
            Layout.margins: 4
            //anchors.fill: parent
            columns: 2
            rows: 2
            Layout.leftMargin: 8
            id: row_delegate


            RowLayout {
                spacing: 0
                // Title/Artist column
                Label {
                    id: queue_label
                    color: it_delegate.isPlaying ? Material.accentColor : Material.foreground
                    property var queue_list: ItemQueue
                    property string queue_str: ""

                    // TODO label width can be wrong when item already in queue and delegate is created
                    // fixed but not opacity
                    Component.onCompleted: {
                        if (queue_list != undefined) {
                            queue_label.text = queue_str
                            console.log("width",  queue_label.implicitWidth)
                            Layout.minimumWidth = queue_label.implicitWidth
                            opacity = 1
                        }
                    }

                    function buildQueueString(queue) {
                        var str_out = "Q: "
                        for (let i = 0; i < queue_list.length; i++) {
                            str_out = str_out + queue_list[i] + ", "
                        }
                        return str_out.slice(0, -2) + " ";
                    }

                    onQueue_listChanged: {
                        if (queue_list != undefined) {
                            queue_str = buildQueueString(queue_list)
                            if (queue_label.text !== queue_str) {
                                previousWidth = queue_label.implicitWidth
                                queue_label.text = queue_str
                                show_animation.start();
                            }
                        }
                        else {
                            Layout.minimumWidth = 0
                            hide_animation.start();
                        }
                    }
                    property int previousWidth: 0
                    ParallelAnimation {
                        id: show_animation
                        PropertyAnimation {
                            target: queue_label
                            property: "Layout.preferredWidth"
                            from: queue_label.previousWidth
                            to: queue_label.implicitWidth
                            duration: 100
                        }
                        PropertyAnimation {
                            target: queue_label
                            property: "opacity"
                            from: queue_label.previousWidth ? 1 : 0
                            to: 1
                            duration: 100
                        }
                    }
                    ParallelAnimation {
                        id: hide_animation
                        PropertyAnimation {
                            target: queue_label
                            property: "Layout.preferredWidth"
                            from: queue_label.implicitWidth
                            to: 0
                            duration: 100
                        }
                        PropertyAnimation {
                            target: queue_label
                            property: "opacity"
                            from: 1
                            to: 0
                            duration: 100
                        }
                        onFinished: {
                            queue_label.text = ""
                        }
                    }
                }

                Label {
                    property string tracknum: ItemTrackNum ? ItemTrackNum + ". " : ""
                    //padding: 2
                    // todo add queue "Q: 1,2" etc.
                    text: tracknum + ItemTitle
                    font.bold: isPlaying
                    color: it_delegate.isPlaying ? Material.accentColor : Material.foreground
                    //width: parent.width
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    //Layout.maximumWidth: implicitWidth
                    clip: true
                }
            }


            Label {
                Layout.alignment: Qt.AlignRight
                padding: 2
                text: ItemCodec
                font.pixelSize: 10
                font.bold: isPlaying
                color: it_delegate.isPlaying ? Material.accentColor : Material.foreground
                Layout.preferredWidth: implicitWidth + 10
                //width: parent.width
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                horizontalAlignment: Text.AlignRight
                elide: Text.ElideRight
                //Layout.fillWidth: true
                //Layout.maximumWidth: implicitWidth
                clip: true
            }

            Label {
                Layout.alignment: Qt.AlignRight
                bottomPadding: 8
                text: ItemArtist + " - " + ItemAlbum
                font.pixelSize: 10
                font.bold: isPlaying
                color: it_delegate.isPlaying ? Material.accentColor : Material.foreground
                //width: parent.width
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                elide: Text.ElideRight
                Layout.fillWidth: true
                //Layout.maximumWidth: implicitWidth
                clip: true
            }

            Label {
                id: time_label
                color: it_delegate.isPlaying ? Material.accentColor : Material.foreground

                bottomPadding: 8
                property string currentTime: ""
                Timer {
                    interval: 100
                    repeat: true
                    running: it_delegate.isPlaying && DBApi.playback.playing
                    onTriggered: {
                        var len = Math.floor(DBApi.playback.position_max * DBApi.playback.position_abs / 100.0)
                        var hour = Math.floor(len/3600)
                        var min = Math.floor((len-hour*3600)/60)
                        var sec = Math.floor((len-hour*3600-min*60))
                        time_label.currentTime = hour ? hour.toString().padStart(2,'0') + ":" + min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0') :
                                             min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0')

                    }
                }
                property string text_stream: it_delegate.isPlaying ? currentTime + "/∞" + ItemLength : "∞"
                property string text_file: it_delegate.isPlaying ? currentTime + "/" + ItemLength : ItemLength
                text: ItemCodec === "content" ? text_stream : text_file
                font.pixelSize: 12
                //width: parent.width
                wrapMode: Text.WrapAnywhere
                maximumLineCount: 1
                elide: Text.ElideRight
                //Layout.fillWidth: true
                //Layout.maximumWidth: implicitWidth
                horizontalAlignment: Text.AlignRight
                Layout.alignment: Qt.AlignRight
                clip: true
            }
        }
        ToolButton {
            id: toolbtn
            icon.name: "overflow-menu"
            Layout.alignment: Qt.AlignVCenter
            //anchors.rightMargin: scrollbar.width
            //Layout.preferredWidth: height
            //anchors.verticalCenter: parent.verticalCenter
            //anchors.verticalCenter: parent.verticalCenter


            onClicked: {
                let menu = menu_component.createObject(this)
                menu.open()
            }

            flat: true
        }
    }
}

/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
