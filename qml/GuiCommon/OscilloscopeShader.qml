import QtQuick 2.15
import QtQuick.Controls 2.12
import QtCharts 2.1
import Qt.labs.platform

import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0

DBWidget {
    friendlyName: qsTr("Oscilloscope Shader")
    internalName: "scopeShader"
    widgetStyle: "Qt Quick"
    widgetType: "main"
    z: 666
    // width: 200
    // height: 200
    widget: ScopeShader {
        //anchors.fill: parent
        id: ss
        Component.onCompleted: {
             DBApi.viz.scope_callback_attach(ss)
        }
        Layout.minimumHeight: 200
        Layout.minimumWidth: 200
        Layout.fillHeight: true
        Layout.fillWidth: true
        Rectangle {
            anchors.fill: parent
            color: "black"
            z: -1
        }

        function changeMode(new_mode) { // Mono / Multichannel
            console.log("DEBUG", ss, ss.config)
            if (ss.config.scope_mode !== new_mode) {
                ss.config.scope_mode = new_mode
                DBApi.conf.set(name_i(), "mode", new_mode)
            }
        }

        function changeFragmentDuration(new_dur) { // [ms]
            if (ss.config.fragment_duration !== new_dur) {
                ss.config.fragment_duration = new_dur
                DBApi.conf.set(name_i(), "fragment_duration", new_dur)
            }
        }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: (mouse) => {
                    if (mouse.button === Qt.RightButton)
                        contextMenu.popup(mouseX,mouseY)
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
                                checked: ss.config.scope_mode === index
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
                            model: [50,55,75, 100,110,200,220,440,300,500]
                            MenuItem {
                                text: qsTr(modelData + " ms")
                                checkable: true
                                checked: ss.config.fragment_duration === modelData
                                onTriggered: {
                                    changeFragmentDuration(modelData)
                                }
                                ActionGroup.group: fragmentGroup
                            }
                            onObjectAdded: frag_menu.insertItem(index, object)
                            onObjectRemoved: frag_menu.removeItem(object)
                        }
                        MenuItem {
                            ToolButton {

                                text: "plus"
                                onClicked: {
                                    ss.config.fragment_duration = ss.config.fragment_duration + 2
                                }
                            }
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
                            model: [0.125, 0.25, 0.5, 0.8, 1.0, 1.2, 2.0,4.0,8.0,16.0,32.0]
                            MenuItem {
                                    text: qsTr(modelData + "x")
                                    checkable: true
                                    checked: ss.config.scale === modelData
                                    onTriggered: {
                                        ss.config.scale = modelData
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
