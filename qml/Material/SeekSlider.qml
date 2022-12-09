import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DBWidget {
    friendlyName: qsTr("Seekbar")
    internalName: "seekSlider"
    widgetStyle: "Qt Quick"
    widgetType: "toolbar"

    Layout.fillWidth: true
    Layout.preferredWidth: 5000

    widget: Slider {
        id: control
        snapMode: Slider.NoSnap
        Layout.minimumWidth: 300
        Layout.preferredWidth: 1000
        Layout.alignment: Qt.AlignVCenter
        padding: 1
        from: 0.0
        to: 100.0
        stepSize: 0.1
        z: 2000

        value: playback.position_rel
        onPressedChanged: {
            // seek on mouse release
            !pressed ? playback.position_abs = value : 0
        }
        enabled: playback.playing || playback.paused

        Timer {
            interval: 10
            running: true
            repeat: true
            onTriggered: {
                if (!pressed)
                    control.value = playback.position_abs
            }
        }


        ToolTip {
            focus: false
            MouseArea {
                acceptedButtons: Qt.NoButton
                hoverEnabled:true
                id: tip_mouse
                width: parent.width * 2
                x: -width/4
                y: -height/2
                height: parent.height * 3
            }

            z: -1
            x: parent.width/2 - width/2
            y: parent.height/2-height/2
            //y: (control.pressed | tip_mouse.containsMouse) ? parent.height : parent.height/2-height/2
            Behavior on y { SmoothedAnimation { velocity: 300; } }
            opacity: (control.hovered && !tip_mouse.containsMouse)//|| control.pressed || tip_mouse.containsMouse)
            Behavior on opacity { SmoothedAnimation { velocity: 3; reversingMode: SmoothedAnimation.Immediate } }
            id: info
            visible: playback.playing && (control.hovered || tip_mouse.containsMouse)// || control.pressed || tip_mouse.containsMouse)
            //visible: (control.hovered || control.pressed | tip_mouse.containsMouse) //&& (parent.MouseArea)
            delay: Qt.styleHints.mousePressAndHoldInterval
            Connections {
                target: control
                function onMoved() {
                    info.delay = 0
                    info.show(info.text)
                    info.delay = Qt.styleHints.mousePressAndHoldInterval
                }
            }

            padding: 2

            property real len: playback.position_max * control.value / 100.0
            property int hour: len/3600
            property int min: (len-hour*3600)/60
            property int sec: (len-hour*3600-min*60)
            text: hour.toString().padStart(2,'0') + ":" + min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0')
        }
    }
}


/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
