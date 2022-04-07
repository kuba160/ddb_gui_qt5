import QtQuick 2.15
import QtQuick.Controls 2.12
import QtCharts 2.1

Item {
    id: main
    readonly property string friendlyName: qsTr("Oscilloscope Polar")
    readonly property string internalName: "oscilloscopePolar"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "main"
    property int instance: -1

    Loader {
        id: loader
        sourceComponent: instance != -1 ? oscilloscope : undefined
        // size determined by rootItem (corresponding to QWidget size)
        width: parent.width
        height: parent.height
        asynchronous: false
    }

    Component {
        id: oscilloscope
        PolarChartView {
            id: chartView
            animationOptions: ChartView.NoAnimation
            legend.visible: false
            anchors.fill: parent

            plotArea: Qt.rect(0,0,width,height)
            plotAreaColor: "black"
            backgroundColor: "black"
            // doesn't work :( - using rectangle
            Rectangle {
                anchors.fill: parent
                color: "black"
                z: -5
            }
           // oscilloscope settings
           property variant scope;

            ValueAxis {
                id: axisRadial
                min: -1
                max: 1
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }
            ValueAxis {
                id: axisAngular
                min: 0
                max: scope.series_width
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }


            Component.onCompleted: {
                scope = api.scope_create(this);
                scope.setSeries(0, series(0))
                scope.setSeries(1, series(1))
                scope.paused = Qt.binding(function() { return !main.visible})
            }

            ScatterSeries {
                id: waveform
                axisAngular: axisAngular
                axisRadial: axisRadial
                useOpenGL: true
                // LineSeries
                //width: 1
                //capStyle: Qt.FlatCap
                // ScatterSeries
                markerShape: ScatterSeries.MarkerShapeRectangle
                markerSize: 1

            }
            ScatterSeries {
                id: waveform2
                axisAngular: axisAngular
                axisRadial: axisRadial
                useOpenGL: true
                // LineSeries
                //width: 1
                //capStyle: Qt.FlatCap
                // ScatterSeries
                markerShape: ScatterSeries.MarkerShapeRectangle
                markerSize: 1

            }

            LineSeries {
                id: waveform3
                axisAngular: axisAngular
                axisRadial: axisRadial
                useOpenGL: true
                // LineSeries
                width: 1
                //capStyle: Qt.FlatCap
            }
            LineSeries {
                id: waveform4
                axisAngular: axisAngular
                axisRadial: axisRadial
                useOpenGL: true
                // LineSeries
                width: 1
                //capStyle: Qt.FlatCap
            }


            MouseArea {
                id: mouse_area
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    if (mouse.button === Qt.RightButton)
                        contextMenu.popup()
                    else if (mouse.button === Qt.LeftButton) {
                        console.log(scope.series_width)
                        contextMenu.dismiss()
                    }
                }
                onPressAndHold: {
                    if (mouse.source === Qt.MouseEventNotSynthesized)
                        contextMenu.popup()
                }

                ActionGroup { id: styleGroup }
                ActionGroup { id: channelGroup }
                ActionGroup { id: fragmentGroup }
                property int style_select: 0
                Menu {
                    id: contextMenu
                    Menu {
                        title: qsTr("Style")
                        Action {
                            text: "Scatter"
                            checkable: true
                            checked: chartView.style_select === 0
                            onTriggered: {
                                scope.setSeries(0, series(0))
                                scope.setSeries(1, series(1))
                                waveform.visible = true
                                waveform2.visible = true
                                waveform3.visible = false
                                waveform4.visible = false
                                mouse_area.style_select = 0
                            }
                            ActionGroup.group: styleGroup
                        }
                        Action {
                            text: "Line"
                            checkable: true
                            checked: chartView.style_select === 1
                            onTriggered: {
                                scope.setSeries(0, series(2))
                                scope.setSeries(1, series(3))
                                waveform.visible = false
                                waveform2.visible = false
                                waveform3.visible = true
                                waveform4.visible = true
                                mouse_area.style_select = 1
                            }
                            ActionGroup.group: styleGroup
                        }
                    }

                    Menu {
                        title: qsTr("Rendering Mode")
                            Action {
                                text: qsTr("Mono")
                                checkable: true
                                checked: scope.scope_mode === 0
                                onTriggered: {
                                    scope.scope_mode = 0
                                }
                                ActionGroup.group: channelGroup
                            }
                            Action {
                                text: qsTr("Multichannel")
                                checkable: true
                                checked: scope.scope_mode === 1
                                onTriggered: {
                                    scope.scope_mode = 1
                                }
                                ActionGroup.group: channelGroup
                            }
                    }
                    Menu {
                        title: qsTr("Fragment Duration")
                            Action {
                                text: qsTr("50 ms")
                                checkable: true
                                checked: true
                                onTriggered: {
                                    scope.fragment_duration = 50
                                }
                                ActionGroup.group: fragmentGroup
                            }
                            Action {
                                text: qsTr("100 ms")
                                checkable: true
                                onTriggered: {
                                    scope.fragment_duration = 100
                                }
                                ActionGroup.group: fragmentGroup
                            }
                            Action {
                                text: qsTr("200 ms")
                                checkable: true
                                onTriggered: {
                                    scope.fragment_duration = 200
                                }
                                ActionGroup.group: fragmentGroup
                            }
                            Action {
                                text: qsTr("300 ms")
                                checkable: true
                                onTriggered: {
                                    scope.fragment_duration = 300
                                }
                                ActionGroup.group: fragmentGroup
                            }
                            Action {
                                text: qsTr("500 ms")
                                checkable: true
                                onTriggered: {
                                    scope.fragment_duration = 500
                                }
                                ActionGroup.group: fragmentGroup
                            }
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
