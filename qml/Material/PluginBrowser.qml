import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DBWidget {
    friendlyName: qsTr("Plugin browser")
    internalName: "pluginBrowserQuick"
    widgetStyle: "DeaDBeeF"
    widgetType: "main"


    Layout.fillWidth: true
    Layout.fillHeight: true

    widget: Item {
        anchors.fill: parent
        id: www
        //Layout.minimumWidth: view.implicitWidth
        //Layout.minimumHeight: view.implicitHeight
        anchors.margins: 20

        Layout.maximumWidth: list.implicitWidth + swipe.implicitWidth

        property alias currentPlugin: list.model
        ListView {
            id: list
            implicitWidth: parent.width / 3
            clip: true
            //anchors.fill: parent
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }


            focus: true
            model: conf.plugins
            header: ItemDelegate {
                    id: t
                    text: "Plugins"
                    enabled: true
                    width: ListView.view.width - scrollbar.width
                    property int sortDirection: -1
                    onClicked: {
                        list.currentIndex = -1
                        conf.plugins.sort(0, !sortDirection)
                        sortDirection = !sortDirection
                    }
                    Layout.fillWidth: true

                    icon.name: sortDirection === 0 ? "view-sort-ascending" :
                               sortDirection === 1 ? "view-sort-descending" :
                                                          undefined
                //icon.name: sortDirection ? "view-sort-ascending" : undefined
            }
            headerPositioning: ListView.OverlayHeader

            delegate: ItemDelegate {
                property string pluginName: PluginName
                property string pluginVersion: PluginVersionMajor + "." + PluginVersionMinor
                property string pluginDescr: PluginDescr
                property string pluginWebsite: PluginWebsite
                property string pluginCopyright: PluginCopyright
                text: pluginName
                width: ListView.view.width - scrollbar.width
                highlighted: index === list.currentIndex
                onClicked: (mouse)=> {
                    list.currentIndex = index
                }
                hoverEnabled: true
                ToolTip.visible: hovered && width < implicitWidth
                ToolTip.delay: 1000
                ToolTip.text: pluginName
            }
            keyNavigationEnabled: true
            ScrollBar.vertical: ScrollBar {
                id: scrollbar
                active: true
            }
        }

        TabBar {
            id: tabs
            currentIndex: swipe.currentIndex
            onCurrentIndexChanged: {
                swipe.currentIndex = currentIndex
            }

            anchors {
                top: parent.top
                left: list.right
                //bottom: swipe.top
                right: parent.right
            }
            height: 50

            TabButton {
                text: "Info"
            }

            TabButton {
                text: "Config"
            }
        }

        Frame {
            anchors {
                top : tabs.bottom
                topMargin: 10
                left: list.right
                leftMargin: 10
                right: parent.right
                bottom: parent.bottom
            }
            SwipeView {
                id: swipe
                clip: true
                anchors.fill: parent

                ScrollView {
                    id: mainview
                    clip: true
                    enabled: true


                    GridLayout {
                        id: grid
                        columnSpacing: 5
                        rowSpacing: 10
                        columns: 2
                        //anchors.fill: parent

                        Label {
                            id: lab1
                            text: "Version:"
                            Layout.alignment: Qt.AlignRight | Qt.AlignTop
                        }

                        TextEdit {
                            Layout.fillWidth: true
                            color: lab1.color
                            readOnly: true
                            selectByMouse: true
                            text: list.currentItem.pluginVersion
                        }

                        Label {
                            Layout.alignment: Qt.AlignRight | Qt.AlignTop
                            text: "Website:"
                        }
                        Label {
                            property string link: list.currentItem.pluginWebsite
                            textFormat: TextEdit.RichText
                            text: '<html><style type="text/css"></style><a href="' + link + '">' + link + '</a></html>'
                            onLinkActivated: Qt.openUrlExternally( link )
                            }

                        Label {
                            id: lab2
                            text: "Description:"
                            Layout.alignment: Qt.AlignRight | Qt.AlignTop
                        }

                        TextEdit {
                            id: edit_text
                            Layout.fillWidth: true
                            color: lab1.color
                            readOnly: true
                            selectByMouse: true
                            text: list.currentItem.pluginDescr

                            Shortcut {
                                sequences: StandardKey.Copy
                                onActivated: edit_text.copy()
                            }


                        }

                        Label {
                            Layout.alignment: Qt.AlignRight | Qt.AlignTop
                            text: "Copyright:"
                        }

                        TextEdit {

                            id: copyright_text
                            Layout.fillWidth: true
                            color: lab1.color
                            readOnly: true
                            selectByMouse: true
                            text: list.currentItem.pluginCopyright
                            //clip:true
                            persistentSelection: true
                            Shortcut {
                                sequences: StandardKey.Copy
                                onActivated: edit_text.copy()
                            }

                            MouseArea {
                                acceptedButtons: Qt.RightButton
                                anchors.fill: parent
                                onClicked: (mouse)=> {
                                   var selectStart = copyright_text.selectionStart;
                                   var selectEnd = copyright_text.selectionEnd;
                                   var curPos = copyright_text.cursorPosition;
                                   contextMenu.popup(mouse.x, mouse.y)
                                   contextMenu.open();
                                   copyright_text.cursorPosition = curPos;
                                   copyright_text.select(selectStart,selectEnd);
                                }
                            }

                            Menu {
                                id: contextMenu
                                title: qsTr("Edit")

                                MenuItem {
                                    text: qsTr("Copy")
                                    enabled: copyright_text.selectedText
                                    onTriggered: copyright_text.copy()
                                }
                                MenuItem {
                                    text: qsTr("Cut")
                                    enabled: false
                                }
                                MenuItem {
                                    text: qsTr("Paste")
                                    enabled: false
                                }
                            }
                        }

                        Item {
                            id: empty
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                        }
                    }


                    ScrollBar.horizontal.policy: width < implicitWidth ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                    ScrollBar.vertical.policy: height < implicitHeight ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
                 }



                Label {
                    text: "TODO"
                }
            }
        }





/*
        Flickable {
            clip: true
            implicitWidth: 100
            Layout.fillHeight: true
            contentWidth: t.width
            contentHeight: t.height
            Text {
                id: t
                text: view.currentItem.pluginDescr
            }
        }
*/

    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
