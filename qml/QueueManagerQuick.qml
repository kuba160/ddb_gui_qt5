import QtQuick 2.15
import QtQuick.Controls 2.12

import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.12

DBWidget {
    friendlyName: qsTr("Queue Manager")
    internalName: "queueManager"
    widgetStyle: "Qt Quick"
    widgetType: "main"

    widget: ColumnLayout {
        anchors.fill: parent
        PlaylistView {
            id : playlistView
            model: playlist.queue
        }
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



/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
