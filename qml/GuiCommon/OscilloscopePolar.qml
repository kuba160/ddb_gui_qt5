import QtQuick 2.15
import QtQuick.Controls 2.12
import QtCharts
import Qt.labs.platform

import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0

DBWidget {
    friendlyName: qsTr("Oscilloscope Polar")
    internalName: "scopePolar"
    widgetStyle: "Qt Quick"
    widgetType: "main"

    widget:        PolarChartView {
        id: chartView
        animationOptions: ChartView.NoAnimation
        legend.visible: false
        width: Math.min(parent.width,parent.height)
        height: Math.min(parent.width,parent.height)
        anchors.fill: parent

        plotArea: Qt.rect(0,0,Math.min(width,height),Math.min(width,height))
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

        function name_i() {
            return instanceName
        }

        ValueAxis {
            id: axisRadial
            min: -1
            max: 1.2 // bigger than 1 to avoid clipping??
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

        Connections {
            id: cons
            target: null
            function onDataChanged() {
                scope.replaceData()
            }
        }

        Component.onCompleted: {
            scope = DBApi.viz.createVisScope(this);
            cons.target = scope
            // settings
            changeStyle(DBApi.conf.get(name_i(), "style", 1))
            scope.mode = DBApi.conf.get(name_i(), "mode", 1)
            scope.fragment_duration = DBApi.conf.get(name_i(), "fragment_duration", 100)
            scope.scale = DBApi.conf.get(name_i(), "scale", 1)

            scope.paused = Qt.binding(function() { return !chartView.visible})

            use_global_accent = DBApi.conf.get(name_i(), "use_global_accent", 1)

            wave1_color = DBApi.conf.get(name_i(), "wave1_color", "#2b7fba")
            wave2_color = DBApi.conf.get(name_i(), "wave2_color", "#2b7fba")

        }

        property color wave1_color: "#2b7fba"
        property color wave2_color: "#2b7fba"
        property int use_global_accent: 1

        ColorDialog {
            id: colorDialog
            title: qsTr("Custom visualization base color")
            property int wave: 0
            onAccepted: {
                if (wave === 1 | wave !== 0) {
                    wave1_color = colorDialog.color
                    DBApi.conf.set(name_i(), "wave2_color", colorDialog.color)
                }
                if (wave === 0 | wave !== 1) {
                    wave2_color = colorDialog.color
                    DBApi.conf.set(name_i(), "wave1_color", colorDialog.color)
                }
            }
        }

        ScatterSeries {
            id: waveform
            axisAngular: axisAngular
            axisRadial: axisRadial
            color: use_global_accent ? "#2b7fba"/*DBApi.accent_color*/ : wave2_color
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
            color: use_global_accent ? "#2b7fba"/*DBApi.accent_color*/ : wave2_color
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
            color: use_global_accent ?"#2b7fba"/*DBApi.accent_color*/ : wave2_color
            useOpenGL: true
            // LineSeries
            //width: 1
            //capStyle: Qt.FlatCap
        }
        LineSeries {
            id: waveform4
            axisAngular: axisAngular
            axisRadial: axisRadial
            color: use_global_accent ? "#2b7fba"/*DBApi.accent_color*/ : wave1_color
            useOpenGL: true
            // LineSeries
            //width: 1
            //capStyle: Qt.FlatCap
        }


        property int style: -1 // Scatter/Line

        function changeStyle(new_style) {
            if (style !== new_style) {
                style = new_style
                if (style === 0) {
                    scope.setSeries(0, series(0))
                    scope.setSeries(1, series(1))
                    waveform.visible = true
                    waveform2.visible = true
                    waveform3.visible = false
                    waveform4.visible = false
                    DBApi.conf.set(name_i(), "style", new_style)
                }
                else {
                    scope.setSeries(0, series(2))
                    scope.setSeries(1, series(3))
                    waveform.visible = false
                    waveform2.visible = false
                    waveform3.visible = true
                    waveform4.visible = true
                    DBApi.conf.set(name_i(), "style", new_style)
                }
            }
        }

        function changeMode(new_mode) { // Mono / Multichannel
            if (scope.scope_mode !== new_mode) {
                scope.scope_mode = new_mode
                DBApi.conf.set(name_i(), "mode", new_mode)
            }
        }

        function changeFragmentDuration(new_dur) { // [ms]
            if (scope.fragment_duration !== new_dur) {
                scope.fragment_duration = new_dur
                DBApi.conf.set(name_i(), "fragment_duration", new_dur)
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: {
                if (mouse.button === Qt.RightButton)
                    contextMenu.open()
                else if (mouse.button === Qt.LeftButton) {
                    //console.log(name_i())
                    contextMenu.close()
                }
            }
            onPressAndHold: {
                if (mouse.source === Qt.MouseEventNotSynthesized)
                    contextMenu.open()
            }

            ActionGroup { id: styleGroup }
            ActionGroup { id: channelGroup }
            ActionGroup { id: fragmentGroup }
            ActionGroup { id: scaleGroup }
            Menu {
                id: contextMenu
                Menu {
                    id: style_menu
                    title: qsTr("Style")

                    Instantiator {
                        model: ["Scatter", "Line"]
                        MenuItem {
                                text: qsTr(modelData)
                                checkable: true
                                checked: chartView.style === index
                                onTriggered: {
                                    changeStyle(index)
                                }
                                ActionGroup.group: styleGroup
                        }
                        onObjectAdded: style_menu.insertItem(index, object)
                        onObjectRemoved: style_menu.removeItem(object)
                    }
                }
                Menu {
                    id: rend_menu
                    title: qsTr("Rendering Mode")
                    Instantiator {
                        model: ["Mono", "Multichannel"]
                        MenuItem {
                            text: qsTr(modelData)
                            checkable: true
                            checked: scope.scope_mode === index
                            onTriggered: {
                                changeMode(index)
                            }
                            ActionGroup.group: channelGroup
                        }
                        onObjectAdded: rend_menu.insertItem(index, object)
                        onObjectRemoved: rend_menu.removeItem(object)
                    }
                }
                Menu {
                    id: frag_menu
                    title: qsTr("Fragment Duration")
                    Instantiator {
                        model: [50,100,200,300,500]
                        MenuItem {
                            text: qsTr(modelData + " ms")
                            checkable: true
                            checked: scope.fragment_duration === modelData
                            onTriggered: {
                                changeFragmentDuration(modelData)
                            }
                            ActionGroup.group: fragmentGroup
                        }
                        onObjectAdded: frag_menu.insertItem(index, object)
                        onObjectRemoved: frag_menu.removeItem(object)
                    }
                }
                Menu {
                    id: color_menu
                    title: "Color"
                    MenuItem {
                        text: "Use global accent color"
                        checkable: true
                        checked: use_global_accent
                        onTriggered: {
                            use_global_accent = use_global_accent ? 0 : 1
                            DBApi.conf.set(name_i(), "use_global_accent", use_global_accent)
                        }
                    }

                    Repeater {
                        model: ["Left channel", "Right channel", "Both"]
                        MenuItem {
                            text: modelData
                            onTriggered: {
                                colorDialog.wave = index
                                colorDialog.open()
                            }
                        }
                    }
                }
                Menu {
                    id: scale_menu
                    title: qsTr("Scale")
                    Instantiator {
                        model: [1,2,4,8,16,32]
                        MenuItem {
                                text: qsTr(modelData + ".0x")
                                checkable: true
                                checked: scope.scale === modelData
                                onTriggered: {
                                    scope.scale = modelData
                                    DBApi.conf.set(name_i(), "scale", modelData)
                                }
                                ActionGroup.group: scaleGroup
                        }
                        onObjectAdded: scale_menu.insertItem(index, object)
                        onObjectRemoved: scale_menu.removeItem(object)
                    }
                }
            }
        }
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 400
    Layout.preferredHeight: 400
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
