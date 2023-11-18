import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import DBApi 1.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.12
import DeaDBeeF.Q.DBApi 1.0

Drawer {
    required property var stack
    clip: true
    id: drawer
    // folded is 42 when style is Fusion, 64 is good for Material
    // TODO fix this shit
    width: Math.max(0.66 * parent.width, 256)
    height: parent.height

    Component {
        id: about_pane
        AboutPane {}
    }

    Component {
        id: equalizer_page
        EqualizerPage {}
    }

    Component {
        id: medialib_pane
        Medialib {}
    }


    Component {
        id: vis_page
       VisPage {}
    }


    ActionGroup {
        id: actions
        exclusive: false
        Action {
            text: "Equalizer"
            icon.name: "view-media-equalizer"
            onTriggered: stack.push(equalizer_page)
        }
        Action {
            text: "Media Library"
            enabled: DBApi.playlist.medialib_available
            icon.name: "view-institution"
            onTriggered: stack.push(medialib_pane)
        }
        Action {
            text: "Visualizations"
            icon.name: "view-media-visualization"
            onTriggered: stack.push(vis_page)
        }
        Action {
            text: "Settings"
            icon.name: "settings-configure"
            onTriggered: stack.push(settings_pane)
        }
        Action {
            text: "About DeaDBeeF"
            icon.name:  "help-about-symbolic"
            icon.color: "transparent"
            onTriggered: stack.push(about_pane)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        ToolBar {
            id: tb
            Material.background: Material.accent
            RowLayout {
                ToolButton {
                    icon.name: "arrow-left"
                    onClicked: {
                        drawer.close()
                    }
                }

                Image {
                    id: logo
                    property int margin: 16
                    Layout.preferredHeight: parent.height - margin
                    Layout.preferredWidth: Layout.preferredHeight
                    source: "/images/simple.svg"
                    smooth: true
                    mipmap: true

                }

                Label {
                    text: "DeaDBeeF"
                    font.bold: true
                    elide: Label.ElideRight
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    Layout.fillWidth: true
                }
            }
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
        }

        ListView {
            id: list
            interactive: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: contentHeight
            Layout.minimumHeight: contentHeight
            width: parent.preferredWidth
            Layout.alignment: Qt.AlignTop
            model: actions.actions
            delegate: ItemDelegate {
                enabled: modelData.enabled
                text: modelData.text
                width: parent.width
                implicitHeight: 42
                //display: width < implicitWidth ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                Layout.alignment: Qt.AlignLeft
                //display: width < implicitWidth ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                icon.name: modelData.icon.name
                //icon.color: modelData.icon.color
                icon.width: 32
                icon.height: 32
                onClicked: {
                    drawer.close()
                    modelData.trigger()
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignBottom
            width: parent.width
            Label {
                id: label_playlists
                Layout.leftMargin: 16
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                font.pixelSize: 18
                text: "Playlists"
            }
            ToolButton {
                Layout.rightMargin: 8
                Layout.alignment: Qt.AlignRight
                icon.name: "list-add"
            }
        }
        PlaylistBrowser {
            drawer: drawer
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}

