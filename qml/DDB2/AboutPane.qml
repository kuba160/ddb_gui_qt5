import QtQuick 2.15
//import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import "."

Page {
    id: mainPane

    property int defaultPane: 0
    property int extraPane: 0

    property list<string> extra_names: ["ChangeLog", "GPLv2", "LGPLv2.1"]
    property list<string> extra_text: [DBApi.conf.aboutChangelog, DBApi.conf.aboutGPLV2,
                             DBApi.conf.aboutLGPLV21]

    Component.onCompleted: {
        console.log("EXTRA:", extraPane)
        if (extraPane > 0) {
            let v = extra_page.createObject(mainPane, {text: extra_text[extraPane-1], title: extra_names[extraPane-1]})
        }
    }

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
            text: "About DeaDBeeF"
            font.pixelSize: 20
            elide: Label.ElideRight
        }
        ToolButton {
            anchors.right: parent.right
            anchors.margins: 5
            icon.name: "overflow-menu"

            Component {
                id: extra_page
                Page {
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
                                if (DDB2Globals.stack.depth > 1) {
                                    DDB2Globals.stack.pop()
                                    this.destroy()
                                }
                            }
                        }

                        Label {
                            anchors.centerIn: parent
                            id: deadbeef_label
                            text: title
                            font.pixelSize: 20
                            elide: Label.ElideRight
                        }
                    }
                    ScrollView {
                        anchors.fill: parent
                        Loader {
                            sourceComponent: ddb_text_view
                            Component.onCompleted: {
                                item.text = text
                            }
                        }
                        ScrollBar.horizontal.policy: width < implicitWidth ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                        ScrollBar.vertical.policy: height < implicitHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff

                    }

                    property string text: undefined
                    Component.onCompleted: {
//                        let v = ddb_text_view.createObject(null, {text:  text})
//                        console.log(DDB2Globals.stack)
                          DDB2Globals.stack.push(this)
                    }

                }
            }

            Component {
                id: extra_menu

                Menu {
                    Repeater {
                        model: 3
                        MenuItem {
                            text: extra_names[index]
                            onTriggered: {
                                let v = extra_page.createObject(mainPane, {text: extra_text[index], title: extra_names[index]})

                            }
                        }
                    }
                    onClosed: this.destroy()
                }
            }

            onClicked: {
                let menu = extra_menu.createObject(this)
                menu.open()

            }
        }
    }

    Component {
        id: ddb_text_view
        ScrollView {
            id: mainview
            clip: true
            enabled: true
            property string text
            property var text_edit: edit_text
            Layout.fillWidth: true
            anchors.centerIn: parent
            //anchors.fill: parent
            ColumnLayout {
                TextEdit {
                    id: edit_text
                    Layout.margins: 16
                    Layout.fillWidth: true
                    color: deadbeef_label.color

                    readOnly: true
                    selectByMouse: true
                    text: mainview.text

                    Shortcut {
                        sequences: [StandardKey.Copy]
                        onActivated: edit_text.copy()
                    }
                }
            }
            ScrollBar.horizontal.policy: width < implicitWidth ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: height < implicitHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
         }
    }

    ColumnLayout {
        anchors.fill: parent
        TabBar {
            width: parent.width
            currentIndex: swipe.currentIndex
            onCurrentIndexChanged: {
                swipe.currentIndex = currentIndex
            }
            Layout.fillWidth: true
            Repeater {
                model: ["DeaDBeeF", "Q User Interface", "Translators"]
                TabButton {
                    text: modelData
                }
            }
        }

        SwipeView {
            id: swipe
            clip: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            currentIndex: defaultPane

            Loader {
                sourceComponent: ddb_text_view
                onLoaded: {
                    item.text = DBApi.conf.aboutText
                }
            }
            Item {
                id: qt_pane
                // Qt User Interface
                Canvas {
                    id: canvas
                    anchors.fill: parent
                    anchors.margins: 8
                    property real lastX
                    property real lastY
                    property int distance: 0
                    onPaint: {
                        var ctx = getContext('2d')
                        ctx.lineWidth = 1.5
                        let px = canvas2.ctx.getImageData(lastX, lastY, 1, 1).data;
                        ctx.strokeStyle =  Qt.rgba(px[0]/255, px[1]/255, px[2]/255, px[3]/255)
                        ctx.beginPath()
                        ctx.moveTo(lastX, lastY)
                        var dx = area.mouseX - lastX
                        var dy = area.mouseY - lastY
                        distance += Math.sqrt(dx * dx + dy * dy)
                        lastX = area.mouseX
                        lastY = area.mouseY
                        ctx.lineTo(lastX, lastY)
                        ctx.stroke()
                    }
                    Label {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.margins: 4
                        text: canvas.distance
                        opacity: 0.01
                    }
                    MouseArea {
                        id: area
                        preventStealing: true
                        anchors.fill: parent
                        onPressed: {
                            canvas.lastX = mouseX
                            canvas.lastY = mouseY
                            canvas2.requestPaint()
                        }
                        onPositionChanged: {
                            canvas.requestPaint()
                        }
                    }
                }
                Canvas {
                    id:canvas2
                    opacity: 0
                    property var ctx
                    anchors.fill: parent
                    anchors.margins: 8
                    onPaint: {
                        ctx = getContext('2d')
                        ctx.drawImage("/images/scalable.svg", 0, 0,
                                      Math.min(width,height),Math.min(width,height))
                    }
                    Component.onCompleted: {
                        loadImage("/images/scalable.svg")
                    }
                }
            }
            Loader {
                sourceComponent: ddb_text_view
                onLoaded: {
                    item.text = DBApi.conf.aboutTranslators
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
