import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0

ListView {
    required property var drawer
    Layout.preferredHeight: view.implicitHeight
    focus: true
    clip: true
    id: view
    model: DBApi.playlist.list
    currentIndex: DBApi.playlist.current_idx
    delegate: ItemDelegate {
        //text: DBApi.playlistName
        width: view.contentWidth
        height: main.implicitHeight + 8
        //highlighted: index === currentIndex

        SystemPalette {
            id: pal
        }

        onClicked: {
            DBApi.playlist.current_idx = index
            drawer.close()
        }

        property bool isCurrent: index === currentIndex
        property color sel_color: isCurrent ? Material.accentColor : pal.text

        RowLayout {
            id: main
            anchors.fill: parent
            ColumnLayout {
                Layout.leftMargin: 16
                Layout.fillWidth: true
                Label {
                    text: playlistName
                    font.bold: isCurrent
                    color: sel_color
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Label {
                    property real len: playlistLength
                    property int hour: len/3600
                    property int min: (len-hour*3600)/60
                    property int sec: (len-hour*3600-min*60)
                    property string length_str: hour ? hour.toString().padStart(2,'0') + ":" + min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0') :
                                     min.toString().padStart(2,'0') + ":" + sec.toString().padStart(2,'0')

                    property string track_str: playlistItems === 1 ? "track" : "tracks"
                    text: playlistItems + " " + track_str + ", " + length_str
                    font.pixelSize: 10
                    font.bold: isCurrent
                    color: sel_color
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            ToolButton {
                Layout.alignment: Qt.AlignRight
                icon.name: "overflow-menu"

                Component {
                    id: menu_playlist
                    ActionMenu {
                        prototype_id: 2
                        itemIterator: ItemIterator
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    z: 10
                    acceptedButtons: Qt.LeftButton
                    onClicked: {
                        console.log("test in")
                        let menu = menu_playlist.createObject(this)
                        menu.open()
                    }
                }
            }
        }
    }
    keyNavigationEnabled: true
    ScrollBar.vertical: ScrollBar {
        id: scrollbar
        active: true
    }
    contentWidth: width - scrollbar.width
}


/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
