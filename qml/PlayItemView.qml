import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


import DBApi 1.0

ListView {
    readonly property string widgetType: "ignore"
    //required property var model
    //required property var delegate
    id: listview
    Layout.fillWidth: true
    Layout.fillHeight: true

    clip: true

    flickDeceleration: 10000
    maximumFlickVelocity: 5000

    reuseItems: true
    spacing: 1

    //Keys.onUpPressed: scrollbar.decrease()
    //Keys.onDownPressed: scrollbar.increase()

    ScrollBar.vertical: ScrollBar {
        id: scrollbar

        active: true
    }

    section.property: "ItemAlbum"
    section.criteria: ViewSection.FullString
    section.delegate: Label {
        text: section
        font.italic: true
        height: implicitHeight + 4
    }
}

/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
