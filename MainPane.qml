import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qml"
import DBApi 1.0
import Qt.labs.settings 1.0
import QtQuick.Controls.Material 2.12

Page {
    id: mainPane
    required property url source
    required property string name
    //anchors.fill: parent
    header: ToolBar {
        //Material.foreground: "white"
        Material.background: Material.accent
        anchors.margins: 5
        //Material.accent: "white"
        ToolButton {
            anchors.margins: 5
            text: "Back"
            icon.name: "go-previous"
            anchors {
                left: parent.left
            }
            anchors.verticalCenter: parent.verticalCenter

            onClicked: {
                if (stack.depth > 1) {
                    stack.pop()
                }
            }

        }

        Label {
            anchors.margins: 5
            id: titleLabel
            text: parent.name
            font.pixelSize: 20
            elide: Label.ElideRight
            anchors.centerIn: parent
        }
    }

    Component.onCompleted: {
        loader.source = mainPane.source
        titleLabel.text = mainPane.name
    }

    Loader {
        id: loader
        anchors.fill: parent
        onLoaded: {
            item.instance = 998
        }
    }
}
