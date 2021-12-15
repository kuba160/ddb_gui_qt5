import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
    property bool expandable: true
    // manual scroll handling
    WheelHandler {
        onWheel: (event)=>
            event.angleDelta.y > 0 ?
                     api.setCurrentPlaylist(tabs.currentIndex - 1) :
                     api.setCurrentPlaylist(tabs.currentIndex + 1)
    }

    TabBar {
        id: tabs
        width: parent.width
        hoverEnabled: true

        // one-way binding, only set currentIndex if current playlist changed
        Binding {
            target: tabs
            property: "currentIndex"
            value: api.current_playlist
        }

        Repeater {
            id: reap
            model: api.playlists

            TabButton {
                text: api.playlists[index]
                width: implicitWidth
                onClicked: {
                    // change playlist, currentIndex will follow
                    api.current_playlist = index
                }
                MouseArea {
                    width: parent.width
                    height: parent.height
                    acceptedButtons: Qt.RightButton
                    onClicked: {
                        api.playlistContextMenu(null, mapToGlobal(mouse.x,mouse.y),index)  //(this,pos,index);
                        rclick.start()
                    }
                }
                SequentialAnimation on down {
                    id: rclick
                    running: false
                    NumberAnimation {
                        from: 0
                        to: 1
                    }
                    PauseAnimation {
                        duration: 1000
                    }
                    NumberAnimation {
                        from: 1
                        to: 0
                    }
                }

            }
            onModelChanged: {
                // model reloaded, reset current index to current playlist
                tabs.currentIndex = api.current_playlist
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
