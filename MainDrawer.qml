import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qml"
import DBApi 1.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.12

Item {
    clip: true
    id: drawer
    // folded is 42 when style is Fusion, 64 is good for Material
    // TODO fix this shit
    Layout.preferredWidth: folded ? 64 :256// Math.max(256, 0.33 * parent.width)
    //Layout.maximumWidth: folded ? 64 : Math.max(256, 0.33 * parent.width)
    //dragMargin: Qt.styleHints.startDragDistance
    Layout.fillHeight: true
    Behavior on Layout.preferredWidth { SmoothedAnimation {id: foldAnimation; velocity: 1000; } }
    property bool folded: true
    property bool iconOnly: true

    Connections {
        target: foldAnimation
        function onFinished() {
            if (drawer.folded) {
                drawer.iconOnly = true
            }
        }
        function onStarted() {
            if (!drawer.folded) {
                drawer.iconOnly = false;
            }
        }
    }

    Component {
        id: plugin_pane
        MainPane {
            source: "qml/PluginBrowser.qml"
            name: "Plugins"
        }
    }
    Component {
        id: widget_pane
        MainPane {
            source: "qml/WidgetBrowser.qml"
            name: "Widget Gallery"
        }
    }
    ActionGroup {
        id: actions_dev
        exclusive: false
        Action {
            text: "Plugins"
            icon.name: "document-properties"
            onTriggered: stack.push(plugin_pane)
        }
        Action {
            text: "Widget Gallery"
            icon.name: "insert-image"
            onTriggered: stack.push(widget_pane)
        }
    }
    Component {
        id: equalizer_pane
        MainPane {
            source: "qml/EqualizerQuick.qml"
            name: "Equalizer"
        }
    }
    ActionGroup {
        id: actions
        exclusive: false
        Action {
            text: "About Qt"
            icon.name:  "help-about-symbolic"
            icon.color: "transparent"
            onTriggered: app.aboutQt()
            enabled: app != undefined
        }
        Action {
            text: "Equalizer"
            icon.name: "view-media-equalizer"
            onTriggered: stack.push(equalizer_pane)
        }
        Action {
            id: action_repeat
            checkable: true
            property var repeat_str: ["Repeat All", "Repeat Off", "Repeat Single"]
            property var repeat_ico: ["media-repeat-all", "media-repeat-none", "media-repeat-single"]
            text: repeat_str[playback.repeat]
            icon.name: repeat_ico[playback.repeat]
            onTriggered: {
                playback.repeat = (playback.repeat+1) % repeat_str.length
            }
        }
        Action {
            id: action_shuffle
            checkable: true
            property var shuffle_str: ["Shuffle Off", "Shuffle Tracks", "Shuffle Random", "Shuffle Albums"]
            property var shuffle_ico: ["media-playlist-normal", "media-random-tracks-amarok", "media-playlist-shuffle", "media-random-albums-amarok"]
            text: shuffle_str[playback.shuffle]
            icon.name: shuffle_ico[playback.shuffle]
            onTriggered: {
                playback.shuffle = (playback.shuffle+1) % shuffle_str.length
            }
        }
        Action {
            text: "Exit"
            icon.name: "application-exit"
            //icon.color: "transparent"
            onTriggered: Qt.quit()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.bottom: parent.bottom
        spacing: 0
        RowLayout {
            anchors.fill: parent
            spacing: 0
            Layout.alignment: Qt.AlignTop
            ItemDelegate {
                clip: true
                //visible: !drawer.folded
                //Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: drawer.folded ? 0 : implicitWidth
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                text: "DeaDBeeF"
                //icon.name: drawer.folded ? "arrow-right" : "arrow-left"
                icon.source: "/images/deadbeef.png"
                icon.color: "transparent"
                icon.width: 32
                icon.height: 32
                implicitHeight: 42
                Behavior on Layout.preferredWidth { SmoothedAnimation {velocity: 1000; } }
                //display: drawer.folded ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                /*ToolButton {
                    //visible: !drawer.folded
                    icon.name: "configure"
                    anchors.right: parent.right
                    enabled: true
                    anchors.verticalCenter: parent.verticalCenter
                }*/
            }
            ItemDelegate {
                clip: true
                //Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                implicitHeight: 42
                //Layout.minimumWidth: drawer.folded ? drawer.preferredWidth : 64
                Layout.maximumWidth: drawer.folded ? parent.width : 64
                icon.name: drawer.folded ? "arrow-right" : "arrow-left"
                //icon.source: !drawer.folded ? "/images/deadbeef.png" : undefined
                //icon.color: "transparent"
                display: AbstractButton.IconOnly
                onClicked: {
                    mainDrawer.folded = !mainDrawer.folded
                }
            }
        }



        ListView {
            id: list_dev
            interactive: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: contentHeight
            Layout.minimumHeight: contentHeight
            width: parent.preferredWidth
            Layout.alignment: Qt.AlignTop
            model: actions_dev.actions
            delegate: ItemDelegate {
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
                    //drawer.close()
                    modelData.trigger(list)
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
        }

        ListView {
            id: list
            interactive: false
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: contentHeight
            Layout.minimumHeight: contentHeight
            width: parent.preferredWidth
            Layout.alignment: Qt.AlignBottom
            model: actions.actions
            delegate: ItemDelegate {
                id: delegate
                clip: true
                text: modelData.text
                implicitWidth: parent.width
                implicitHeight: 42
                //width: parent.width
                //display: drawer.iconOnly ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                //Layout.alignment: Qt.AlignLeft
                icon.name: modelData.icon.name
                //icon.color: modelData.icon.color
                icon.width: 32
                icon.height: 32
                enabled: modelData.enabled
                onClicked: (mouse)=> {
                    if (!modelData.checkable)
                        ;
                        //drawer.folded = true
                    else
                        click_animation.restart()
                    modelData.trigger(list)
                }
                onDoubleClicked: (mouse)=> {
                    //mouse.accepted = false
                    if (modelData.checkable) {
                        onClicked(mouse)
                        click_animation.restart()
                    }
                }
                Connections {
                    target: foldAnimation
                    function onFinished() {
                        display = drawer.folded ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                    }
                }
                hoverEnabled: true
                ToolTip.visible: drawer.folded && hovered
                ToolTip.text: delegate.text
                ToolTip.delay: 500

                SequentialAnimation {
                    id: click_animation
                    ScaleAnimator {
                        target: delegate
                        from: 1;
                        to: 1.15;
                        duration: 1000
                    }
                    ScaleAnimator {
                        target: delegate
                        from: 1.15;
                        to: 1;
                        duration: 1000
                    }
                }
            }
        }
    }
}

