import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
    readonly property string friendlyName: qsTr("Tab strip")
    readonly property string internalName: "tabBarQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "toolbar"
    property int instance: -1

    Loader {
        id: loader
        sourceComponent: instance >= 0 ? tabBar : undefined
        // size determined by rootItem (corresponding to QWidget size)
        width: parent.width
        height: parent.height
    }
    Component {
        id: tabBar
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
                anchors.fill: parent

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
                        text: playlistName
                        width: implicitWidth
                        height: parent.height
                        padding: 5
                        horizontalPadding: 2*padding
                        anchors.bottom: parent.bottom
                        onClicked: {
                            // change playlist, currentIndex will follow
                            api.current_playlist = index
                        }
                        Keys.onLeftPressed: {
                            api.current_playlist = tabs.currentIndex - 1
                        }
                        Keys.onRightPressed: {
                            api.current_playlist = tabs.currentIndex + 1
                        }
                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.RightButton
                            onClicked: (mouse)=> {
                                api.playlistContextMenu(null, mapToGlobal(mouse.x,mouse.y),index)  //(this,pos,index);
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
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
