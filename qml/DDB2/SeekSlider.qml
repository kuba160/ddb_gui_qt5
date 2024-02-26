import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DBWidget {
    friendlyName: qsTr("Seekbar")
    internalName: "seekSlider"
    widgetStyle: "Qt Quick"
    widgetType: "toolbar"

    Layout.fillWidth: true
    Layout.fillHeight: false
    Layout.preferredWidth: 500
    //Layout.preferredHeight: row.implicitHeight

    widget: Item {
        property real time_current_abs: 0
        implicitHeight: row.implicitHeight
        Layout.maximumHeight: row.implicitHeight
        Layout.fillWidth: true

        Timer {
            interval: 10
            running: DBApi.playback.playing
            repeat: true
            onTriggered: {
                time_current_abs = DBApi.playback.position_abs
            }
        }

        Component {
            id: time_label
            Label {
                padding: 5
                property bool remaining: false
                property bool remaining_total: DBApi.conf.get("DDB2","SeekSlider_remaining_total", 0)
                property real len: remaining && !remaining_total ? Math.abs((control.value - 100.0)) * DBApi.playback.position_max / 100.0 :
                                    remaining ? DBApi.playback.position_max
                                             : DBApi.playback.position_max * control.value / 100.0
                property int hour: len/3600
                property int min: (len-hour*3600)/60
                property int sec: (len-hour*3600-min*60)
                property string minus: remaining && !remaining_total ? "-" : ""

                text: hour ? minus + hour.toString().padStart(2,'0') + ":" + min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0') :
                             minus + min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0')
                font.pixelSize: 10

                MouseArea {
                    enabled: parent.remaining
                    anchors.fill: parent
                    onPressAndHold: {
                        DBApi.conf.set("DDB2","SeekSlider_remaining_total", !parent.remaining_total)
                        parent.remaining_total = !parent.remaining_total
                    }
                }
            }
        }
        Rectangle {
            anchors.fill: parent
            color: Material.color(Material.Grey, Material.Shade800)
            //opacity: 0.1
            z: -5
        }
        RowLayout {
            id: row
            visible: DBApi.playback.playing || DBApi.playback.paused
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            //height: control.implicitHeight

            Loader {
                sourceComponent: time_label
                onLoaded: {
                    item.remaining = false
                }
            }

            Slider {
                id: control
                snapMode: Slider.NoSnap
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                padding: 1
                from: 0.0
                to: 100.0
                stepSize: 0.1
                z: 2000


                Binding on value {
                    when: !control.pressed
                    value: time_current_abs
                }
                Behavior on value { SmoothedAnimation { velocity: 250; } }

                onPressedChanged: {
                    // seek on mouse release
                    !pressed ? DBApi.playback.position_abs = value : 0
                    !pressed ? time_current_abs = value : 0
                }
                enabled: DBApi.playback.playing || DBApi.playback.paused
                Canvas {
                    id: line_canvas
                    anchors.fill: parent
                    contextType: "2d"
                    opacity: 0.75
                    property bool stop_after_current: DBApi.playback.stop_after_current
                    property bool stop_after_album: DBApi.playback.stop_after_album

                    onStop_after_albumChanged: requestPaint()
                    onStop_after_currentChanged: requestPaint()

                    onPaint: {
                        var context = getContext("2d");
                        context.clearRect(0,0,line_canvas.width, line_canvas.height)
                        // mid line
                        if (stop_after_album || stop_after_current) {
                            context.strokeStyle = Material.accent //Qt.rgba(1,1,1,.5);
                            if (stop_after_album) {
                                context.setLineDash([2,2])
                            }
                            else {
                                context.setLineDash([])
                            }

                            context.lineWidth = 1
                            context.path = undefined
                            context.moveTo(width, 2) //line_canvas.height/2)
                            context.lineTo(width, line_canvas.height - 2);
                            context.stroke()
                        }
                    }
                    z: 2
                }
            }
            Loader {
                sourceComponent: time_label
                onLoaded: {
                    item.remaining = true
                }
            }
        }
        RowLayout {
            anchors.fill: parent
            Layout.fillWidth: true
            Layout.preferredHeight: row.implicitHeight
            Label {
                anchors.centerIn: parent
                visible: DBApi.playback.stopped
                //Layout.alignment: Qt.AlignCenter
                text: "Nothing is playing"
            }
        }
    }
}


/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
