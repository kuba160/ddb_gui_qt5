import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DBWidget {
    friendlyName: qsTr("Tab strip")
    internalName: "tabBar"
    widgetStyle: "Qt Quick"
    widgetType: "toolbar"

    // height determined by tab size
    //height: loader.height
    // width determined by parent (context)
    //Layout.fillWidth: true
    Layout.fillHeight: false

    widget: TabBar {
        clip: true
        //width: parent.width
        Layout.minimumHeight: implicitHeight
        Layout.maximumHeight: implicitHeight
        //Layout.alignment: Qt.AlignBottom
        id: tabs
        currentIndex: playlist.current_idx
        onCurrentIndexChanged: {
            playlist.current_idx = currentIndex
        }
        background: MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: {
                if (wheel.pixelDelta.y < 0 || wheel.angleDelta.y < 0) {
                    tabs.currentIndex = Math.min(tabs.currentIndex + 1, tabs.contentModel.count -1);
                } else {
                    tabs.currentIndex = Math.max(tabs.currentIndex - 1, 0);
                }
            }
            /*acceptedButtons: Qt.RightButton
            onClicked: (mouse)=> {
                api.playlistContextMenu(null, mapToGlobal(mouse.x,mouse.y),index)  //(this,pos,index);
            }*/
        }
        Repeater {
            id: reap
            model: playlist.list
            TabButton {
                text: playlistName
                width: implicitWidth
                height: implicitHeight + 5
                anchors.bottom: parent.bottom
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
