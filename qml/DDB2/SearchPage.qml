import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DDB2Page {
    title: "Search"

    Component.onCompleted: {
        search_line.forceActiveFocus();
    }

    ColumnLayout {
        anchors.fill: parent

        TextField {
            id: search_line
            //Layout.margins: 2
            padding: 8
            Layout.margins: 8
            Layout.fillWidth: true
            Layout.maximumHeight: parent.implicitHeight
            placeholderText: "Search music"
            text: DBApi.playlist.current_search.item_filter

            onTextChanged: {
                DBApi.playlist.current_search.item_filter = text
            }

            RoundButton {
                text: "×"
                width: height
                flat: true
                visible: search_line.text.length
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                    top: parent.top
                    margins: 4
                }
                onClicked: {
                    search_line.clear()
                }
            }
        }

        Item {
            //anchors.fill: parent
            Layout.fillHeight: true
            Layout.fillWidth: true
            PlayItemView {
                id: playlist_search
                focus: false
                anchors.fill: parent
                model: DBApi.playlist.current_search
                delegate: DDB2PlayItemViewDelegate {}
            }
            ItemDelegate {
                anchors.centerIn: parent
                text: search_line.length > 0 ?
                          "No results found" :
                          "Use search field to find tracks."
                visible: search_line.length <= 0 || !playlist_search.count
                opacity: visible ? 1 : 0
                enabled: false
            }
        }
    }
}
