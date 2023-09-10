#include "DBActionMenu.h"
#include <QDebug>
#include <QActionGroup>

void actionUpdate(QAction *action, QHash<QString, QVariant> prop_map) {
    if (prop_map.contains("enabled")) {
        action->setEnabled(prop_map.value("enabled").toBool());
    }
    if (prop_map.contains("hide") && prop_map.value("hide").toBool()) {
        action->setVisible(false);
    }
    if (prop_map.contains("checkable") && prop_map.value("checkable").toBool()) {
        action->setCheckable(true);
    }
    if (prop_map.contains("checked")) {
        action->setChecked(prop_map.value("checked").toBool());
    }
    if (prop_map.contains("icon")) {
        action->setIcon(QIcon::fromTheme(prop_map.value("icon").toString()));
    }
    if (prop_map.contains("text")) {
        action->setText(prop_map.value("text").toString());
    }
}

void buildMenuIter(QMenu *parent, DBApi *Api, QJsonArray a, PlayItemIterator pit) {
    for (int i = 0; i < a.size(); i++) {
        QJsonObject obj = a.at(i).toObject();
        if (obj.value("type").toString() == "submenu") {
            QMenu *menu = parent->addMenu(obj.value("text").toString());
            buildMenuIter(menu, Api, obj.value("children").toArray(), pit);
        }
        else if (obj.value("type").toString() == "action") {
            QAction *action = parent->addAction(obj.value("text").toString());
            QString action_id = obj.value("id").toString();
            action->setProperty("action_id", action_id);
            action->connect(action, &QAction::triggered, [action, Api, pit] {
                // none
                Api->actions.execAction(action->property("action_id").toString(), pit);
            });
            action->connect(Api->actions.getAction(action_id), &DBAction::actionPropertiesChanged, [action, action_id, Api] () {
                // TODO playitemiterator
                PlayItemIterator pit;
                QHash<QString, QVariant> properties = Api->actions.getActionContext(action_id, pit);
                actionUpdate(action, properties);
            });

            if (obj.contains("properties")) {
                QJsonObject prop_map = obj.value("properties").toObject();
                // update standard
                actionUpdate(action, prop_map.toVariantHash());

                if (prop_map.contains("separator_before") && prop_map.value("separator_before").toBool()) {
                    parent->insertSeparator(action);
                }
                if (prop_map.contains("separator_after") && prop_map.value("separator_after").toBool()) {
                    parent->addSeparator();
                }
                if (prop_map.contains("exclusive_group")) {
                    QString group = prop_map.value("exclusive_group").toString();
                    QHash<QString, QActionGroup *> groups = parent->property("exclusive_groups").value<QHash<QString, QActionGroup *>>();
                    QActionGroup *ag = nullptr;
                    if (groups.contains(group)) {
                        ag = groups.value(group);
                    }
                    else {
                        ag = new QActionGroup(parent);
                        groups.insert(group, ag);
                    }
                    ag->addAction(action);

                    parent->setProperty("exclusive_groups", QVariant::fromValue(groups));
                }
            }
        }
    }
}

QMenuBar* buildMenuBar(QWidget *parent, DBApi *Api) {
    QMenuBar *menu = new QMenuBar(parent);
    PlayItemIterator pit;
    // todo custom menu bar
    QJsonArray array = Api->actions.parsePrototype(3, pit);

    for (int i = 0; i < array.size(); i++) {
        QJsonObject obj = array.at(i).toObject();
        if (obj.value("type") == "submenu") {
            QMenu *submenu = menu->addMenu(obj.value("text").toString());
            buildMenuIter(submenu, Api, obj.value("children").toArray(), pit);
        }
    }
    return menu;
}


QMenu* buildTrackContextMenu(QWidget *parent, DBApi *Api, PlayItemIterator pit) {
    QMenu *menu = new QMenu(parent);
    QJsonArray array = Api->actions.parsePrototype(1, pit);
    buildMenuIter(menu, Api, array, pit);
    return menu;
}
