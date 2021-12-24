import QtQuick 2.12
import QtQuick.Controls 2.12

Slider {
    id: control
    property bool expandable: true
    padding: 1
    from: 0.0
    to: 100.0
    stepSize: 0.1

    value: api.position
    onPressedChanged: {
        // seek on mouse release
        !pressed ? api.position = value : 0
    }
    enabled: api.playing || api.paused

    Timer {
        interval: 10
        running: true
        repeat: true
        onTriggered: {
            if (!pressed)
                control.value = api.position
        }
    }

    background: Item {
        id: base
        width: control.width - 2*control.padding
        height: parent.height
        anchors.fill: parent
        property int h: 10
        property int rad: 10
        property int bw: 2
        property color c: "#2b7fba"
        Rectangle {
            z: 0
            width: parent.width
            anchors.centerIn: parent
            height: base.h
            radius: base.rad
            color: "transparent"
            border.color: base.c
            border.width: base.bw
        }
        Item {
            z: 1
            anchors.left: parent.left
            clip:true
            width: parent.width * control.value/100.0
            height: parent.height
            Behavior on width { SmoothedAnimation { velocity: 4000 } }
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: base.width
                height: base.h
                radius: base.rad
                color: base.c
            }
        }
        // Info box
        Rectangle {
            id: info
            anchors.centerIn: parent
            radius: 4
            color: "#2b7fba"
            z: 100
            Text {
                id: text
                z: 101
                anchors.centerIn: parent
                property real len: api.playing_length * control.value / 100.0
                property int hour: len/3600
                property int min: (len-hour*3600)/60
                property int sec: (len-hour*3600-min*60)
                text: hour.toString().padStart(2,'0') + ":" + min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0')
                color: "white"
                font.bold: true
                font.pointSize: 14
            }
            width: text.width + 20
            height: text.height - 2

            state: control.pressed ? "pressed" : "default"
            states: [
                State {
                    name: "default"
                },
                State {
                    name: "pressed"
                }
            ]
            property real opac: 0.8
            opacity: 0
            transitions: [
                Transition {
                    from: "default"; to: "pressed"
                    OpacityAnimator {
                        target: info
                        from: 0; to: info.opac;
                        duration: 10
                    }
                },
                Transition {
                    from: "pressed"; to: "default"
                    SequentialAnimation {
                        PauseAnimation {
                            duration: 500
                        }
                        OpacityAnimator {
                            target: info
                            from: info.opac; to: 0;
                            duration: 500
                        }
                    }
                }
            ]
        }

        /*
        Rectangle {
            x: parent.width * control.value/100.0 + 1
            anchors.verticalCenter: parent.verticalCenter
            height: base.h
            color: "red"
            opacity: (parent.width * control.value/100.0) - (x-1)
        }*/

    }
    // Dummy handle
    handle: Item {
        implicitWidth: 1
        implicitHeight: 1
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
