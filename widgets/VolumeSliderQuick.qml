import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    readonly property string friendlyName: qsTr("Volume bar")
    readonly property string internalName: "volumeSliderQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "toolbar"
    property int instance: -1

    Loader {
        id: loader
        sourceComponent: instance >= 0 ? volumeSlider : undefined
        // size determined by rootItem (corresponding to QWidget size)
        width: parent.width
        height: parent.height
    }
    Component {
        id: volumeSlider
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
                anchors.centerIn: parent

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
                ToolTip {
                    id: info
                    visible: control.hovered || control.pressed
                    delay: Qt.styleHints.mousePressAndHoldInterval
                    Connections {
                        target: control
                        function onMoved() {
                            info.delay = 0
                            info.show(control.value + "dB")
                            info.delay = Qt.styleHints.mousePressAndHoldInterval
                        }
                    }

                    padding: 2
                    z: 10
                    text: control.value + "dB"
                    MouseArea {
                        anchors.fill: parent
                        onPressed: (event)=> {
                                   info.close()
                                   var p = mapToItem(control,event.x, event.y)
                                   api.volume = control.valueAt((p.x-control.leftPadding)/control.availableWidth)
                        }
                        onWheel: (event)=> event.angleDelta.y < 0 ?
                                     api.volume = api.volume - 1 :
                                     api.volume = api.volume + 1
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

            // Dummy handle
            handle: Item {
                implicitWidth: 1
                implicitHeight: 1
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
