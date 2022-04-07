import QtQuick 2.15
import QtQuick.Controls 2.12

import Qt.labs.qmlmodels 1.0

Item {
    readonly property string friendlyName: qsTr("Album Art")
    readonly property string internalName: "artworkQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "main"
    property int instance: -1

    Loader {
        id: loader
        sourceComponent: instance >= 0 ? coverart : undefined
        // size determined by rootItem (corresponding to QWidget size)
        width: parent.width
        height: parent.height
    }
    Component {
        id: coverart
        Image {
            fillMode: Image.PreserveAspectFit
            cache: false
            property int current: 0
            source: api.current_cover_url

        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
