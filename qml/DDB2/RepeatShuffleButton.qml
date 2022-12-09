import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import DBApi 1.0
import Qt.labs.settings 1.0

import QtQuick.Controls.Material 2.12

ToolButton {
    required property Action button_action
    id: repeat_shuffle_tool_button
    action: button_action

    display: AbstractButton.IconOnly
    icon.width: 32
    icon.height: 32

    ToolTip.delay: 500
    ToolTip.timeout: 2000
    ToolTip.visible: hovered || pressed
    ToolTip.text: text
    onPressedChanged: {
        ToolTip.toolTip.show(text)
    }
    onClicked: {
        click_animation.restart()
    }

    SequentialAnimation {
        id: click_animation
        ScaleAnimator {
            target: repeat_shuffle_tool_button
            from: 1;
            to: 0.8;
            duration: 750
        }
        ScaleAnimator {
            target: repeat_shuffle_tool_button
            from: 0.8;
            to: 1;
            duration: 750
        }
    }
}
