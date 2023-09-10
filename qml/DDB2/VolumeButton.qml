import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import DeaDBeeF.Q.DBApi 1.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.12

ToolButton {
    //required property Action button_action
    id: volume_tool_button
    action: button_action

    MouseArea {
        anchors.fill: parent
        onWheel: (wheel) => {
            console.log("wheel", wheel.angleDelta)
            if (wheel.angleDelta.y > 0) {
                console.log("+1")
                DBApi.playback.volume = DBApi.playback.volume + 1
            }
            if (wheel.angleDelta.y < 0) {
                console.log("-1")
                DBApi.playback.volume = DBApi.playback.volume - 1
            }
         }

        onClicked: {
            click_animation.restart()
            if (volume_menu.visible) {
                volume_menu.close()
            }
            else {
                volume_menu.popup(volume_tool_button.parent, volume_tool_button.parent.width - volume_menu.width - menu_popup_margin , 0-volume_menu.height - menu_popup_margin)
            }
        }
        onPressAndHold: {
            click_animation.restart()
            if (DBApi.playback.volume != -50) {
                volume_last_restore = DBApi.playback.volume
                DBApi.playback.volume = -50
            }
            else {
                DBApi.playback.volume = volume_last_restore
            }
        }

    }

    function volume_icon(db) {
        if (db <= -50) {
            return "audio-volume-muted"
        }
        else if (db < -30) {
            return "audio-volume-low"
        }
        else if (db < -10) {
            return "audio-volume-medium"
        }
        else {
            return "audio-volume-high"
        }
    }

    display: AbstractButton.IconOnly
    icon.name: volume_icon(DBApi.playback.volume)
    icon.width: 24
    icon.height: 24



    readonly property int menu_popup_margin: 8
    property real volume_last_restore: -10

    Menu {
        id: volume_menu

        Slider {
            padding: 16
            from: -50
            to: 0
            stepSize: 1
            value: DBApi.playback.volume

            onMoved: {
                DBApi.playback.volume = value
            }
            wheelEnabled: true

            ToolTip.visible: pressed
            ToolTip.text:  "%1dB".arg(DBApi.playback.volume)
        }
    }

    SequentialAnimation {
        id: click_animation
        ScaleAnimator {
            target: volume_tool_button
            from: 1;
            to: 0.8;
            duration: 750
        }
        ScaleAnimator {
            target: volume_tool_button
            from: 0.8;
            to: 1;
            duration: 750
        }
    }
}
