import QtQuick 2.15
//import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0

Page {
    id: mainPane

    //anchors.fill: parent
    header: ToolBar {
        //Material.foreground: "white"
        Material.background: Material.accent
        anchors.margins: 5
        //Material.accent: "white"
        ToolButton {
            anchors.left: parent.left
            anchors.margins: 5
            text: "Back"
            icon.name: "go-previous"

            onClicked: {
                if (stack.depth > 1) {
                    stack.pop()
                }
            }
        }

        Label {
            anchors.centerIn: parent
            id: deadbeef_label
            text: "Equalizer"
            font.pixelSize: 20
            elide: Label.ElideRight
        }
        ToolButton {
            anchors.right: parent.right
            icon.name: "overflow-menu"
            onClicked: {
                menu.open()
            }

            Menu {
                id: menu
                MenuItem {
                    text: qsTr("Zero All")
                    onTriggered: {
                        DBApi.eq.values = Array(DBApi.eq.values.length)
                    }
                }

                MenuItem {
                    text: qsTr("Zero Preamp")
                    onTriggered: {
                        let vals = DBApi.eq.values
                        vals[0] = 0
                        DBApi.eq.values = vals
                    }
                }
                MenuItem {
                    text: qsTr("Zero Bands")
                    onClicked: {
                        let new_values =  Array(DBApi.eq.values.length)
                        new_values[0] = DBApi.eq.values[0]
                        DBApi.eq.values = new_values
                    }
                }
            }

        }
    }

    ColumnLayout {
        Layout.bottomMargin: 5
        anchors.fill: parent
        RowLayout {
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            id: conf_row
            Layout.fillWidth: true
            // controls
            ComboBox {
                implicitContentWidthPolicy: ComboBox.WidestText
                editable: currentIndex < 6 ? false : true
                model: ListModel {
                    id: model
                    ListElement { text: "Off" }
                    ListElement { text: "Balanced" }
                    ListElement { text: "Bass punch" }
                    ListElement { text: "Brilliant treble" }
                    ListElement { text: "Extreme bass" }
                    ListElement { text: "Vocalizer" }
                }
                displayText: "Custom"
            }
            Button {
                text: "Save"
            }
            Item {
                Layout.fillWidth: true
                height: 10
            }

            Switch {
                text: checked ? "ON" : "OFF"
                Layout.alignment: Qt.AlignRight
                checked: DBApi.eq.enabled
                onCheckedChanged: {
                    DBApi.eq.enabled = !DBApi.eq.enabled
                }
            }
        }
        Item {
            id: rangeslider
            Layout.fillWidth: true
            height: conf_row.height
            width: parent.width
            // swipe box
            Rectangle {
                id: full
                anchors.fill: parent
                color: Material.Grey
                opacity: 0.1
            }

            Label {
                anchors {
                    top: parent.top
                    left: parent.left
                }
                anchors.margins: 1

                font.pixelSize: 10
                text: "+20 dB"
            }

            Label {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                }
                anchors.margins: 1
                font.pixelSize: 10
                text: "-20 dB"
            }


            MouseArea {
                id: area
                anchors.fill: parent

            }
            Rectangle {
                id: rectslider
                width: flick.visibleArea.widthRatio * full.width
                height: parent.height
                color: Material.accentColor
                opacity: 0.1
                onXChanged: {
                    flick.contentX = x / flick.visibleArea.widthRatio
                }
                Binding on x {
                    restoreMode: Binding.RestoreNone
                    when: area.pressed
                    value: Math.min(Math.max(0,area.mouseX - rectslider.width/2), full.width-rectslider.width)
                }
            }

            Canvas {
                id: line_canvas
                anchors.fill: parent
                contextType: "2d"

                onPaint: {
                    context.clearRect(0,0,line_canvas.width, line_canvas.height)
                    // mid line
                    context.strokeStyle = Qt.rgba(1,1,1,.5);
                    context.setLineDash([6,4])
                    context.lineWidth = 1
                    context.path = undefined
                    context.moveTo(0,line_canvas.height/2)
                    context.lineTo(line_canvas.width,line_canvas.height/2);
                    context.stroke()
                }
                z: 2
            }


            Canvas {
                id: canvas
                anchors.fill: parent
                contextType: "2d"

                readonly property int slider_count: DBApi.eq.values.length

                function xFromIdx(idx : int) : int {
                    let margin = 5
                    return (idx/(canvas.slider_count-2))*(canvas.width-4*margin) + 2*margin
                }

                function yFromValue(value : int) : int {
                    // from -20 to 20 to [0,height]
                    let margin = 5
                    let height = canvas.height - 2 * margin
                    return height - (value + 20) * height / 40 + margin
                }

                Path {
                    id: myPath
                    startX: canvas.xFromIdx(0); startY: canvas.yFromValue(DBApi.eq.values[1])
                }

                Instantiator {
                    model: canvas.slider_count-1
                    onObjectAdded: (index, object) => { myPath.pathElements.push(object) }
                    PathCurve {
                        x: canvas.xFromIdx(index)
                        y: canvas.yFromValue(DBApi.eq.values[index+1])
                        onYChanged: {
                            canvas.requestPaint()
                        }
                    }
                }

                onHeightChanged: canvas.requestPaint()
                onWidthChanged: canvas.requestPaint()


                onPaint: {
                    context.clearRect(0,0,canvas.width, canvas.height)

                    context.strokeStyle = Material.accentColor;
                    context.lineWidth = 2
                    context.path = myPath;
                    context.stroke();
                }
                z: 5
            }
        }

        Flickable {
            id: flick
            Layout.fillHeight: true
            Layout.fillWidth: true
            // sliders
            contentWidth: Math.max(row.implicitWidth, width)
            contentHeight: height - 5
            clip: true
            interactive: false
            RowLayout {
                id: row
                anchors.fill: parent
                Repeater {
                    id: reap
                    model: canvas.slider_count
                    ColumnLayout {
                        id: column
                        property alias sliderValue: slid.value
                        height: parent.height
                        //Layout.fillHeight: true
                        Slider {
                            id: slid
                            Layout.fillHeight: true
                            //implicitHeight: -1
                            //Layout.preferredHeight: flick.height - label.height
                            //height: flick.height - label.height
                            orientation: Qt.Vertical
                            from: -20.0
                            to: 20.0

                            value: DBApi.eq.values[index]
                            onMoved: {
                                let vals = DBApi.eq.values
                                vals[index] = value
                                DBApi.eq.values = vals
                            }
                            stepSize: Math.abs(value) < 1.1 ? 1 : 0.0
                            snapMode: Slider.SnapAlways
                        }
                        Label {
                            id: label
                            text: DBApi.eq.names[index]
                            Layout.alignment: Qt.AlignHCenter
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
