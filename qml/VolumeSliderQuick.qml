import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

DBWidget {
    friendlyName: qsTr("Volume bar")
    internalName: "volumeSlider"
    widgetStyle: "Qt Quick"
    widgetType: "toolbar"

    Layout.maximumWidth: 200
    Layout.fillHeight: true

    widget: Slider {
        Layout.alignment: Qt.AlignVCenter
        id: control
        //padding: 6
        from: -50
        to: 0
        stepSize: 1

        // bind to api
        value: playback.volume
        onMoved: {
            playback.volume = value
        }
        wheelEnabled: true
            // Volume text info

        ToolTip {
            x: parent.width/2 - width/2
            y: parent.height
            id: info
            visible: control.hovered || control.pressed
            delay: Qt.styleHints.mousePressAndHoldInterval
            Connections {
                target: control
                function onMoved() {
                    info.delay = 0
                    info.show( Math.round(control.value) + "dB")
                    info.delay = Qt.styleHints.mousePressAndHoldInterval
                }
            }
            padding: 2
            z: 10
            text: Math.round(control.value) + "dB"
            MouseArea {
                //anchors.fill: parent
                onPressed: (event)=> {
                           info.close()
                           var p = mapToItem(control,event.x, event.y)
                           api.volume = control.valueAt((p.x-control.leftPadding)/control.availableWidth)
                }
                onWheel: (event)=> event.angleDelta.y < 0 ?
                             api.volume = api.volume + 1 :
                             api.volume = api.volume - 1
            }
            opacity: 0

            enter: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                }
            }
            exit: Transition {
                SequentialAnimation {
                    PauseAnimation {
                        duration: 200
                    }
                    NumberAnimation {
                        property: "opacity"
                        from: 1.0
                        to: 0.0
                        easing.type: Easing.InCubic
                    }
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
