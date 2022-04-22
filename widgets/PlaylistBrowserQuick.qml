import QtQuick 2.15
import QtQuick.Controls 2.12

Item {
    readonly property string friendlyName: qsTr("Playlist browser")
    readonly property string internalName: "playlistBrowserQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "main"
    property int instance

    Loader {
        id: loader
        sourceComponent: api === null ? undefined : playlistBrowser
        // size determined by rootItem (corresponding to QWidget size)
        width: parent.width
        height: parent.height
    }
    Component {
        id: playlistBrowser
        ListView {
            id: view
            model: api.playlists
            delegate: ItemDelegate {
                text: playlistName
                width: loader.width
                highlighted: index === api.current_playlist
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: (mouse)=> {
                        if (mouse.button === Qt.RightButton)
                            api.playlistContextMenu(null, mapToGlobal(mouse.x,mouse.y),index)  //(this,pos,index);
                        else
                            api.current_playlist = index
                    }
                }
            }
            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
