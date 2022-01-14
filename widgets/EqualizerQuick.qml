import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
    id: main
    readonly property string friendlyName: qsTr("Equalizer")
    readonly property string internalName: "equalizerQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "main"
    property int instance: -1

    Loader {
        id: loader
        sourceComponent: instance != -1 ? equalizer : undefined
        // size determined by rootItem (corresponding to QWidget size)
        width: parent.width
        height: parent.height
        asynchronous: false
    }
    Component {
        id: equalizer
        Column {
            id: column
            spacing: 2
            Row {
                id: row_buttons
                width: parent.width
                spacing: 4
                padding: 2
                CheckBox {
                    text: qsTr("Enable")
                    onClicked: {
                        api.eq_enabled = checked
                    }
                    checked: api.eq_enabled
                    padding: 3
                }
                Button {
                    text: qsTr("Zero All")
                    onClicked: {
                        api.eq = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
                    }
                }
                Button {
                    id: preamp
                    text: qsTr("Zero Preamp")
                    onClicked: {
                        var eq = api.eq
                        eq[0] = 0
                        api.eq = eq
                    }
                }
                Button {
                    text: qsTr("Zero Bands")
                    onClicked: {
                        api.eq = [api.eq[0],0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
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
                        value: api.eq[0]
                        onMoved: {
                            var eq = api.eq
                            eq[0] = value
                            api.eq = eq
                        }
                    }
                    Label {
                        id: amp_text
                        text: qsTr("preamp")
                        font.pixelSize: 10
                        color: palette.text
                        bottomPadding: 8
                        anchors.top: amp.bottom
                    }
                }
                Item {
                    id: item1
                    width: 32
                    height: parent.height
                    Label {
                        id: text_high
                        text: amp.value == -20.0 ? Math.round((amp.value+20) * 10)/10 + "dB" : "+" + Math.round((amp.value+20) * 10)/10 + "dB"
                        font.pixelSize: 10
                        anchors.top: parent.top
                        anchors.left: parent.left
                        color: palette.text
                    }
                    Label {
                        id: text_low
                        text: Math.round((amp.value-20) * 10)/10 + "dB"
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        bottomPadding: 20
                        font.pixelSize: 10
                        color: palette.text
                    }
                    Label {
                        id: text_curr
                        y: marea.mouseY - height/2
                        visible: marea.pressed
                        text: Math.round(api.eq[marea.current_slider+1] * 10)/10 + "dB"
                        font.pixelSize: 10
                        color: palette.text
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
                            property var freq_list: ["55 Hz", "77 Hz", "110 Hz", "156 Hz", "220 Hz", "311 Hz", "440 Hz", "622 Hz", "880 Hz", "1.2 kHz", "1.8 kHz", "2.5 kHz", "3.5 kHz", "5 kHz", "7 kHz", "10 kHz", "14 kHz", "20 kHz"]
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
                                    value: api.eq[index+1]
                                    anchors.horizontalCenter: column_eq.horizontalCenter
                                    onMoved: {
                                        var eq = api.eq
                                        eq[index+1] = value
                                        api.eq = eq
                                    }
                                }
                                Label {
                                    id: text_band
                                    width: slider_band.width
                                    text: repeater.freq_list[index]
                                    font.pixelSize: 10
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignTop
                                    anchors.top: slider_band.bottom
                                    anchors.horizontalCenter: column_eq.horizontalCenter
                                    bottomPadding: 8
                                    color: palette.text
                                }
                            }
                        }
                    }
                    MouseArea {
                        id: marea
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        property int current_slider: mouseX/(width/repeater.count)
                        onMouseYChanged: {
                            if (pressed) {
                                if (current_slider >= 0 && current_slider < 18) {
                                    // calculate position based on mouse location (todo: not most accurate method)
                                    var pos = -1 * repeater.itemAt(current_slider).column_slider.valueAt(mouseY/repeater.itemAt(current_slider).column_slider.height)
                                    // replace value at current_slider
                                    var eq = api.eq
                                    eq[current_slider+1] = pos
                                    api.eq = eq
                                }
                            }
                        }
                    SystemPalette { id: palette; colorGroup: SystemPalette.Active }
                    }
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
