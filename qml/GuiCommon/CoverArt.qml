import QtQuick 2.12
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.12
import QtQml 2.12

import DeaDBeeF.Q.DBApi 1.0

DBWidget {
    friendlyName: qsTr("Cover Art")
    internalName: "coverArt"
    widgetStyle: "Qt Quick"
    widgetType: "main"
    Layout.fillHeight: true
    Layout.fillWidth: true
    Layout.minimumHeight: 32
    Layout.minimumWidth: 32

    widget: component

    Component {
        id: component
        Image {
            mipmap: true
            smooth: false
            asynchronous: true
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.maximumWidth: Math.min(height,width)
            Layout.maximumHeight: Math.min(height,width)
            fillMode: Image.PreserveAspectFit
            source: DBApi.playlist.current_cover_path
        }
    }
}
