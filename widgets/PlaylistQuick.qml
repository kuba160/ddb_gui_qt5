import QtQuick 2.15
import QtQuick.Controls 2.12

import DBApi.PlaylistModel 1.0

Item {
    width: 400
    height:400
    Text{
        text: "1234567"
    }

    PlaylistModel {
        id: playlist_model
        plt_num: api.current_playlist
    }

    TableView {
        id: table
        model: playlist_model
    }
}


/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
