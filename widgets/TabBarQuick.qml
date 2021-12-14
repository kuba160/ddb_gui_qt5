import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
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

/* dumb container mode
Container {
    id: container
    WheelHandler {
        onWheel: (event)=>
            event.angleDelta.y > 0 ? container.decrementCurrentIndex() : container.incrementCurrentIndex()
    }


    contentItem: ListView {
        model: container.contentModel
        snapMode: ListView.SnapOneItem
        orientation: ListView.Horizontal
    }

    currentIndex: api.current_playlist
    onCurrentIndexChanged: {
        api.current_playlist = currentIndex
        console.log(currentIndex)
        reap.itemAt(currentIndex).pressed()
    }


    Repeater {
        id: reap
        model: api.playlists

        TabButton {
            text: api.playlists[index]
            width: implicitWidth
            onClicked: {
                container.setCurrentIndex(index)
            }
            function switch() {
                clicked
            }
        }
    }

    ButtonGroup {
        id: bg
        buttons: reap.children
    }
}
*/
/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/