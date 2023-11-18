import QtQuick 2.15
//import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import "."

Item {
    required property var objs
    required property list<string> titles
    property int index_default: 0


    id: swipe_view_tab
    Layout.fillHeight: true
    Layout.fillWidth: true

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
                model: swipe_view_tab.titles
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
            currentIndex: index_default

            Repeater {
                model: objs
                Loader {
                    sourceComponent: modelData
                    enabled: true
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
