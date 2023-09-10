import QtQuick 2.15
import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0
import QtQuick.Controls.Material 2.15
import Qt.labs.qmlmodels

Item {
    required property int prototype_id
    required property var itemIterator
    id: menu_track
    clip: true
    implicitWidth: 0

    function exportProperty(modelData, prop) {
        if (modelData.hasOwnProperty('properties') &&
                modelData.properties.hasOwnProperty(prop)) {
                return modelData.properties[prop];
        }
        return undefined
    }

    function exportProperty2(modelData, prop, def) {
        if (modelData.hasOwnProperty('properties') &&
                modelData.properties.hasOwnProperty(prop)) {
                return modelData.properties[prop];
        }
        return def
    }



    Component {
        id: menu_item
        MenuItem {
            required property var modelData
            font.pointSize: 10
            autoExclusive: exportProperty(modelData, "exclusive_group") !== undefined ? true : false
            text: exportProperty(modelData, "text") ? exportProperty(modelData, "text") : modelData.text
            enabled: exportProperty(modelData, "enabled") !== undefined ? exportProperty(modelData, "enabled") : true
            icon.name: exportProperty(modelData, "icon")
            checkable: exportProperty2(modelData, "checkable", false)
            checked: exportProperty2(modelData, "checked", false)
            onTriggered: {
                DBApi.actions.execAction(modelData.id, itemIterator)
            }

            Component.onCompleted: {
                parent.implicitWidth = Math.max(parent.implicitWidth, implicitWidth)
            }
        }
    }

    Component {
        id: menu_separator
        MenuSeparator {}
    }

    Component {
        id: menu_group
        ButtonGroup {
        }
    }

    Component {
        id: menu_iter
        Menu {
            required property var modelData

            property var group_items
            function separator() {
                addItem(menu_separator.createObject(this))
            }

            function populateMenu() {
                group_items = {}
                let l = modelData
                for (let i = 0; i < l.length; i++) {
                    if (l[i].type === "submenu") {
                        let sub = menu_iter.createObject(this, {title: l[i].text, modelData: l[i].children})
                        sub.populateMenu()
                        addMenu(sub)
                    }
                    else {

                        if (exportProperty(l[i], "separator_before") === true)
                            separator()
                        let sub = menu_item.createObject(this, {modelData: l[i]})
                        addItem(sub)
                        if (exportProperty(l[i], "separator_after") === true)
                            separator()
                        if (exportProperty(l[i], "exclusive_group")) {
                            let group = exportProperty(l[i], "exclusive_group")
                            if (!group_items.hasOwnProperty(group)) {
                                group_items[group] = menu_group.createObject(this)
                            }
                            group_items[group].addButton(sub)
                        }
                    }
                }
            }
        }
    }
    property var root: undefined

    Component.onCompleted: {
        let l = DBApi.actions.parsePrototype(prototype_id, itemIterator)
        root = menu_iter.createObject(this, { modelData: l})
        root.populateMenu()
        root.closed.connect(root.destroy)
    }
    function open() {
        root.open()
    }
}
