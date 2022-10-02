import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

DBWidget {
    friendlyName: qsTr("Playlist browser")
    internalName: "playlistBrowserQuick"
    widgetStyle: "DeaDBeeF"
    widgetType: "main"

    clip: true

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.minimumWidth: 400


    widget: ListView {
        Layout.preferredHeight: view.implicitHeight
        focus: true
        id: view
        model: playlist.list
        currentIndex: playlist.current_idx
        delegate: ItemDelegate {
            text: playlistName
            width: ListView.view.width
            highlighted: index === currentIndex
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: (mouse)=> {
                    if (mouse.button === Qt.RightButton)
                        api.playlistContextMenu(null, mapToGlobal(mouse.x,mouse.y),index)  //(this,pos,index);
                    else if (mouse.button === Qt.LeftButton)
                        playlist.current_idx = index
                }
            }
        }
        keyNavigationEnabled: true
        ScrollBar.vertical: ScrollBar {
            id: scrollbar
            active: true
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
