import QtQuick
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.12
import QtQml 2.12

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DBWidget {
    friendlyName: qsTr("Actions Tree")
    internalName: "actionsTree"
    widgetStyle: "Qt Quick"
    widgetType: "main"
    Layout.fillHeight: true
    Layout.fillWidth: true

    widget: component

    Component {
        id: component

        TreeView {
            Component.onCompleted: {
                console.log(DBApi.actions.actions)
            }

            anchors.fill: parent
            //Layout.fillHeight: true
            //Layout.fillWidth: true
            model: DBApi.actions.actions

            delegate: Item {
                id: treeDelegate

                implicitWidth: padding + label.x + label.implicitWidth + padding
                implicitHeight: label.implicitHeight * 1.5

                readonly property real indent: 20
                readonly property real padding: 5

                // Assigned to by TreeView:
                required property TreeView treeView
                required property bool isTreeNode
                required property bool expanded
                required property int hasChildren
                required property int depth

                TapHandler {
                    onTapped: treeView.toggleExpanded(row)
                }

                Text {
                    id: indicator
                    visible: treeDelegate.isTreeNode && treeDelegate.hasChildren
                    x: padding + (treeDelegate.depth * treeDelegate.indent)
                    anchors.verticalCenter: label.verticalCenter
                    text: "▸"
                    rotation: treeDelegate.expanded ? 90 : 0
                }

                Text {
                    id: label
                    x: padding + (treeDelegate.isTreeNode ? (treeDelegate.depth + 1) * treeDelegate.indent : 0)
                    width: treeDelegate.width - treeDelegate.padding - x
                    clip: true
                    text: model.display
                }
            }
        }
    }
}
