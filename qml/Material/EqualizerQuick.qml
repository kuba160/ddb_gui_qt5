import QtQuick 2.15
import QtQuick.Controls 2.12

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DBWidget {
    friendlyName: qsTr("Equalizer")
    internalName: "equalizer"
    widgetStyle: "Qt Quick"
    widgetType: "main"

/*
    Text {
        id: bg
        anchors.fill: parent
        Rectangle {
            visible: overrideBg
            color: overrideBg ? bg.palette.window : "black"
            anchors.fill: parent
        }
    }*/

    widget: Column {
        id: column
        spacing: 2
        Row {
            id: row_buttons
            width: parent.width
            spacing: 12
            padding: 4
            leftPadding: 16
            CheckBox {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Enable")
                onClicked: {
                    eq.enabled = checked
                }
                checked: eq.enabled
                padding: 3
            }
            Button {
                text: qsTr("Zero All")
                onClicked: {

                    eq.values = Array(eq.values.length)
                }
            }
            Button {
                id: preamp
                text: qsTr("Zero Preamp")
                onClicked: {
                    var vals = eq.values
                    vals[0] = 0
                    eq.values = vals
                }
            }
            Button {
                text: qsTr("Zero Bands")
                onClicked: {
                    var new_values =  Array(eq.values.length)
                    new_values[0] = eq.values[0]
                    eq.values = new_values
                }
            }
            Button {
                text: qsTr("Presets")
            }
        }
        Row {
            id: row_sliders
            width: parent.width
            height: parent.height - row_buttons.height
            padding: 2
            layer.enabled: false
            Item {
                id: column_amp
                width: Math.max(amp.width, amp_text.width)
                height: parent.height
                Slider {
                    id: amp
                    height: parent.height - amp_text.height
                    wheelEnabled: false
                    stepSize: 0.1
                    to: 20
                    from: -20
                    orientation: Qt.Vertical
                    anchors.horizontalCenter: parent.horizontalCenter
                    value: eq.values[0]
                    onMoved: {
                        var vals = eq.values
                        vals[0] = value
                        eq.values = vals
                    }
                    Behavior on value { SmoothedAnimation { velocity: 250; } }
                }
                Label {
                    id: amp_text
                    text: eq.names[0]
                    font.pixelSize: 10
                    //color: palette.text
                    bottomPadding: 8
                    anchors.top: amp.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
            Item {
                clip: true
                id: item1
                width: 38
                height: parent.height
                Label {
                    id: text_high
                    text: amp.value == -20.0 ? Math.round((amp.value+20) * 10)/10 + "dB" : "+" + Math.round((amp.value+20) * 10)/10 + "dB"
                    font.pixelSize: 10
                    anchors.top: parent.top
                    anchors.left: parent.left
                    //color: palette.text
                    visible: text_curr.visible ? text_curr.y > y + height : true
                }
                Label {
                    id: text_low
                    text: Math.round((amp.value-20) * 10)/10 + "dB"
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    bottomPadding: 20
                    font.pixelSize: 10
                    //color: palette.text
                    visible: text_curr.visible ? text_curr.y + text_curr.height < y : true
                }
                Label {
                    id: text_curr
                    anchors.right: text_low.right
                    //anchors.horizontalCenter: parent.horizontalCenter
                    y: amp.height/2 - height/2
                    visible: (marea.containsMouse || marea.pressed) && (marea.current_slider >= 0 && marea.current_slider < 18)
                    color: marea.containsMouse ? text_low.color : text_low.color
                    text: Math.round(eq.values[marea.current_slider+1] * 10)/10 + "dB"
                    font.pixelSize: 10
                    //color: palette.text
                }
            }
            Item {
                width: row_sliders.width - column_amp.width - item1.width
                height: parent.height
                Row {
                    anchors.fill: parent
                    Repeater {
                        id: repeater
                        anchors.fill: parent
                        baselineOffset: 0
                        model: 18
                        Item {
                            id: column_eq
                            property alias column_slider: slider_band
                            x: 448
                            y: 0
                            width: repeater.width/repeater.count
                            height: parent.height
                            Slider {
                                id: slider_band
                                from: -20
                                to: 20
                                stepSize: 0.1
                                height: parent.height - text_band.height
                                width: parent.width
                                orientation: Qt.Vertical
                                value: eq.values[index+1]
                                anchors.horizontalCenter: column_eq.horizontalCenter
                                onMoved: {
                                    var vals = eq.values
                                    vals[index+1] = value
                                    eq.values = vals
                                }
                                Behavior on value { SmoothedAnimation { velocity: 250; } }
                            }
                            Label {
                                id: text_band
                                property bool isKhz: index >= 9
                                property string shortText: eq.names[index+1].split(' ')[0] + (isKhz ? "k" : "")
                                width: slider_band.width
                                text: width >= 40 ? eq.names[index+1] : shortText
                                font.pixelSize: 10
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignTop
                                anchors.top: slider_band.bottom
                                anchors.horizontalCenter: column_eq.horizontalCenter
                                bottomPadding: 8
                                //color: palette.text
                            }
                        }
                    }
                }
                MouseArea {
                    id: marea
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    property int current_slider: mouseX/(width/repeater.count)
                    onMouseYChanged: {
                        if (pressed) {
                            if (current_slider >= 0 && current_slider < 18) {
                                // calculate position based on mouse location (todo: not most accurate method)
                                var pos = -1 * repeater.itemAt(current_slider).column_slider.valueAt(mouseY/repeater.itemAt(current_slider).column_slider.height)
                                // replace value at current_slider
                                var vals = eq.values
                                vals[current_slider+1] = pos
                                eq.values = vals
                            }
                        }
                    }
                SystemPalette { id: palette; colorGroup: SystemPalette.Active }
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorColor:"#c0c0c0";formeditorZoom:1.25;height:480;width:640}
D{i:1}D{i:2}
}
##^##*/
