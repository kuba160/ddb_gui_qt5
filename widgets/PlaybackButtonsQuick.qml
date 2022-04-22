import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    readonly property string friendlyName: qsTr("Playback controls")
    readonly property string internalName: "playbackButtonsQuick"
    readonly property string widgetStyle: "DeaDBeeF"
    readonly property string widgetType: "toolbar"
    property int instance

    // size determined by playbackControls size
    width: loader.width; height: loader.height
    Loader {
        id: loader
        sourceComponent: api === null ? undefined : playbackControls
    }

    Component {
        id: playbackControls
        Row {
            id: button_row
            spacing: 2
            property var names: ["Stop", "Play", "Pause", "Previous", "Next"]
            property var icons: ["media-playback-stop", "media-playback-start", "media-playback-pause", "media-skip-backward", "media-skip-forward"]
            property var actions: [api.stop, api.play, api.togglePause, api.playPrev, api.playNext]
            Repeater {
                id: reap
                model: names
                Button {
                    icon.name: icons[index]
                    icon.color: "white"
                    text: names[index]
                    display: AbstractButton.IconOnly
                    onClicked: {
                        actions[index]()
                    }
                    flat: true
                    width: 32
                    height: width
                }
            }
            width: reap.count * (32 + spacing)
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
