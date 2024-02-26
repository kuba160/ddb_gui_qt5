import QtQuick
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DDB2Page {
    id: page
    title: "Queue"

    page_menu: Menu {
        MenuItem {
            text: "Clear queue"
            icon.name: "edit-clear-all"
            onClicked: {
                DBApi.playlist.queue.removeRows(0,DBApi.playlist.queue.rowCount())
            }
        }
    }

    Item {
        anchors.fill: parent
        PlayItemView {
            id: piv
            anchors.fill: parent
            model: DBApi.playlist.queue
            delegate: FocusScope {
                width: parent.width; height: childrenRect.height
                x:childrenRect.x; y: childrenRect.y
                DDB2PlayItemViewDelegate {}}

            ItemDelegate {
                anchors.centerIn: parent
                text: "Queue is empty"
                visible: piv.count === 0
                opacity: visible ? 1 : 0
                enabled: false
            }
        }
    }
}
