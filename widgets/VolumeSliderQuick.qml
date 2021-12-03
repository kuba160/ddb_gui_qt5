import QtQuick 2.12
import QtQuick.Controls 2.12

Slider {
    id: control
    value: 0.5
    padding: 5
    background: Row {
        spacing: 1
        x: control.leftPadding
        Repeater {
            id: repeat
            model: 20
            Rectangle {
                id: rect
                anchors {
                    bottom: parent.bottom
                }
                width: (control.availableWidth)/repeat.count
                height: control.height*(index/repeat.count)
                color: "green"
            }
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: 26
        implicitHeight: 26
        radius: 13
        color: control.pressed ? "#f0f0f0" : "#f6f6f6"
        border.color: "#bdbebf"
    }
}
