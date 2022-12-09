import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DBWidget {
    friendlyName: qsTr("Widget browser")
    internalName: "widgetBrowser"
    widgetStyle: "Qt Quick"
    widgetType: "main"

    clip: true
    widget: RowLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.minimumWidth: view.implicitWidth
        Layout.minimumHeight: view.implicitHeight
        ListView {
            Layout.fillHeight: true
            Layout.minimumWidth: 250
            Layout.preferredWidth: 250
            //anchors.fill: parent
            focus: true
            id: view
            model: plugin.list
            property var currentQmlUrls: view.currentItem.pluginQmlUrls
            delegate: ItemDelegate {
                property string pluginName: WidgetFriendlyName
                property var pluginStyles: WidgetStyles
                property var pluginQmlUrls: WidgetQmlUrls
                property var pluginTypes: WidgetTypes
                text: pluginName
                width: ListView.view.width
                highlighted: index === view.currentIndex
                onClicked: (mouse)=> {
                    view.currentIndex = index
                }
                Layout.fillHeight: true
            }
            keyNavigationEnabled: true
            ScrollBar.vertical: ScrollBar {
                id: scrollbar
                active: true
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignTop
            Layout.fillHeight: true
            Layout.fillWidth: true
            property alias selection: tabs_styles.currentIndex
            property url qmlUrl: view.currentItem.pluginStyles.length > 1 ? view.currentQmlUrls[selection] : view.currentQmlUrls
            property bool qmlIsMain: view.currentItem.pluginTypes.length > 1 ?  view.currentItem.pluginTypes[selection] == "main" : view.currentItem.pluginTypes == "main"
            TabBar {

                Layout.fillWidth: true
                id: tabs_styles
                Repeater {
                    model: view.currentItem.pluginStyles
                    TabButton {
                        text: modelData
                        width: implicitWidth
                    }
                }
            }
            Loader {
                clip: true

                Layout.fillHeight: parent.qmlIsMain ? true : false
                Layout.fillWidth: parent.qmlIsMain ? true : false

                source: view.currentItem.pluginStyles.length > 1 ? view.currentQmlUrls[parent.selection] : view.currentQmlUrls
                onLoaded: {
                    item.instance = 999
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
