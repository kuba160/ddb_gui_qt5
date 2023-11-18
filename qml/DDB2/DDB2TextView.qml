import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0


ScrollView {
    id: mainview
    clip: true
    enabled: true
    required property string text
    anchors.fill: parent

    ColumnLayout {
        TextEdit {
            id: edit_text
            Layout.margins: 16
            Layout.fillWidth: true

            Label {
                id: dumb_label
                text: ""
            }
            color: dumb_label.color

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
