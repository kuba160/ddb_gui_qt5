import QtQuick 2.15
import QtQuick.Controls 2.12

import Qt.labs.qmlmodels 1.0

Item {
    readonly property string friendlyName: qsTr("Playlist")
    readonly property string internalName: "playlistQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "main"
    property int instance: -1

    Loader {
        id: loader
        sourceComponent: instance >= 0 ? playlist : undefined
        // size determined by rootItem (corresponding to QWidget size)
        width: Math.max(parent.width, 200)
        height: parent.height
    }
    Component {
        id: playlist
        PlaylistView {
            model: api.current_playlist_model
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
