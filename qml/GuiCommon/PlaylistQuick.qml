import QtQuick 2.15
import QtQuick.Controls 2.12

import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0

DBWidget {
    friendlyName: qsTr("Playlist")
    internalName: "playlist"
    widgetStyle: "Qt Quick"
    widgetType: "main"

    widget: PlayItemView {
        id : playlistView
        model: DBApi.playlist.current
        delegate: PlayItemViewDelegate {}
    }

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredWidth: 400
    Layout.preferredHeight: 400
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
