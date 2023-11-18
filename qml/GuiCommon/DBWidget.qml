import QtQuick 2.15

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.12

Item {
    id: root
    required property string friendlyName
    required property string internalName
    property string widgetStyle: "Default"
    property string widgetType: "toolbar"
    readonly property string widgetFormat: "qml"
    property int instance
    required property Component widget
    property string instanceName: internalName + instance

    Layout.fillHeight: true
    Layout.fillWidth: true
    Material.accent: Material.Teal
    Material.theme: Material.Dark

    Rectangle {
        anchors.fill: parent
        visible: _db_bg_override
        color: _db_bg
        z: -5
    }

    implicitHeight: loader.implicitHeight
    implicitWidth: loader.implicitWidth

    Component.onCompleted: {
/*
        if (root.parent !== undefined && root.parent.instance !== undefined) {
            if (root.parent.instance > 0)
                ;//instance = root.parent.instance
        }*/
    }

    //RowLayout {
        //anchors.fill: parent
        //Layout.fillHeight: true
        //Layout.fillWidth: true
        //Layout.margins: 2
        Loader {
            anchors.fill: parent
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: loader
            active: _db_do_not_load ? false : true
            sourceComponent: root.instance >=0 ? root.widget : undefined

            property string dberror: ""
            onSourceComponentChanged: {
                if (friendlyName.length === 0) {
                    dberror = "friendlyName undefined"

                }
                else if (internalName.length == 0) {
                    dberror = "internalName undefined"
                }
                else if (widgetType !== "toolbar" && widgetType !== "main" && widgetType !== "ignore") {
                    dberror = "widgetType undefined"
                }

                if (dberror.length > 0) {
                    console.log("Failed to load DBWidget: " + dberror)
                    sourceComponent = errorLabel
                }
            }
            Component {
                id: errorLabel
                Label {
                    text: "Error: " + parent.dberror
                }
            }
        }
    //}
}


/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
