import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

DBWidget {
    friendlyName: qsTr("Playback controls")
    internalName: "playbackButtons"
    widgetStyle: "Qt Quick"
    widgetType: "toolbar"

    Layout.minimumWidth: implicitWidth

    // size determined by playbackControls size
    //Layout.fillHeight: true
    //Layout.minimumWidth:

    //width: widget !== undefined ? widget.width : 0
    //Layout.minimumWidth: widget.implicitWidth
    //Layout.minimumHeight: widget.implicitHeight
    //width: widget.width; height: widget.height

    widget: RowLayout {
        id: button_row
        Layout.margins: 5
        Layout.fillHeight: false
        Layout.fillWidth: false
        //width: Layout.preferredWidth
        Layout.minimumWidth: implicitWidth
        Layout.maximumWidth: preferredWidth
        //Layout.preferredWidth: reap
        property var names: ["Stop", "Play", "Pause", "Previous", "Next"]
        property var icons: ["media-playback-stop", "media-playback-start", "media-playback-pause", "media-skip-backward", "media-skip-forward"]
        property var actions: [playback.stop, playback.play, playback.pause, playback.prev, playback.next]
        Repeater {
            id: reap
            model: names
            Button {
                icon.name: icons[index]
                //icon.color: "white"
                text: names[index]
                display: AbstractButton.IconOnly
                onClicked: {
                    actions[index]()
                }
                flat: true
                //implicitWidth: implicitHeight
                implicitWidth: implicitHeight
                icon.width: 20
                //L
                //width: height
                //Layout.maximumWidth: height
                //Layout.minimumWidth: 16
                //Layout.minimumHeight: 16

                //width: 32
                //height: width
            }
        }

    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
