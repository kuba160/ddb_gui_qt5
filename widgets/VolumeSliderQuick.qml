import QtQuick 2.12
import QtQuick.Controls 2.12

Slider {
    id: control
    padding: 4
    bottomPadding: 6
    topPadding: 6
    height: 28
    implicitWidth: 76
    // volume range / step size
    from: -50
    to: 0
    stepSize: 1

    // bind to api
    value: api.volume
    onMoved: {
        api.volume = value
    }
    wheelEnabled: true
    // Selection drawing
    background: Item {
        id: base
        width: control.width - control.leftPadding - control.rightPadding
        height: control.height - control.topPadding - control.bottomPadding
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        // Bars properties
        property int minHeight: 3
        property int maxHeight: height
        property int barWidth: 3
        property int barSpacing: 1

        // calculations
        property int barAmount: width > 0 ? (width)/(barWidth + barSpacing) : 0
        // NOTE: the formula below is not correct for other ranges :(
        property real valueNormalized: -(control.value - (control.from - control.to)) / (control.from-control.to)
        property int selectedBarAmount: Math.round(valueNormalized * barAmount)
        function barHeight(bar, total, minheight, maxheight) {
            return (minheight) + (bar/total)*(maxheight-minheight)
        }

        // Bars
        Row {
            id: row_bg
            anchors.bottom: parent.bottom
            spacing: base.barSpacing
            z: 1
            Repeater {
                // blank bars
                model: base.barAmount
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: base.barWidth
                    height: base.barHeight(index, base.barAmount, base.minHeight, base.maxHeight)
                    color: index < base.selectedBarAmount ? "#2b7fba" : "#c0d9eb"
                }
            }
        }
        // "Passthrough" line (no amp)
        Canvas {
            id: passthrough_line
            width: base.barAmount * (base.barWidth + base.barSpacing) - base.barSpacing
            height: parent.height
            z: 0
            onPaint: {
                var ctx = getContext("2d");
                ctx.strokeStyle = Qt.rgba(1,1,1,1);
                ctx.setLineDash([6,4])
                ctx.lineWidth = 1
                ctx.moveTo(0,0)
                ctx.lineTo(width,0);
                ctx.stroke()
            }
            visible: control.value == 0
        }
        // Volume text info
        Rectangle {
            id: info
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            color: "white"
            border.color: "black"
            border.width: 1
            z: 100
            Text {
                id: text
                z: 101
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                text: value + "dB"
                color: "black"
            }
            width: text.width + 5
            height: text.height

            state: control.pressed ? "pressed" : "default"
            states: [
                State {
                    name: "default"
                },
                State {
                    name: "pressed"
                }
            ]
            opacity: 0
            transitions: [
                Transition {
                    from: "default"; to: "pressed"
                    OpacityAnimator {
                        target: info
                        from: 0; to: 1;
                        duration: 10
                    }
                },
                Transition {
                    from: "pressed"; to: "default"
                    OpacityAnimator {
                        target: info
                        from: 1; to: 0;
                        easing.type: Easing.InCubic
                        duration: 1000
                    }
                }
            ]
        }
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
