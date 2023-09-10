#include "Actions.h"

#include "models/ActionsModel.h"

#include "ActionsDefault.h"

#define DBAPI (this->deadbeef)

Actions::Actions(QObject *parent)
    : QObject{parent} {

    //default_actions = new ActionsDefault(this, Api);

    QList<DBAction::ActionLocations> filter_flags = { DBAction::ACTION_LOC_HOTKEY, DBAction::ACTION_LOC_TRACK_CONTEXT,
                                DBAction::ACTION_LOC_PLAYLIST_CONTEXT, DBAction::ACTION_LOC_MENUBAR };

    for (int i = 0 ; i < 4; i++) {
        //m_prototypes.insert(i, new ActionsModel(this, i, getDefaultConfig(filter_flags[i])));
        registerPrototype(static_cast<uint32_t>(filter_flags[i]), DBAction::ACTION_ARG_ALL, getDefaultConfig(filter_flags[i]));
        //m_prototypes.insert(i, new ActionsModel(this, i));
    }

}

void Actions::registerActionOwner(ActionOwner *owner) {
    m_action_owners.append(owner);
    emit actionsAdded();
//    for (ActionsModel *model : m_prototypes.values()) {
//        model->registerActionOwner(owner);
//    }
}

// unregister action owner

uint32_t Actions::registerPrototype(uint32_t location_filter, uint32_t accepts_filter, QString config) {
    m_prototypes.insert(prototype_counter, new ActionsModel(this, location_filter, config));
    return prototype_counter++;
}


QJsonArray Actions::parsePrototype(uint32_t prototype, PlayItemIterator pit) {
    if (m_prototypes.contains(prototype)) {
        QJsonObject root = m_prototypes.value(prototype)->toJson({}, pit);
        if (root.contains("children")) {
            return root.value("children").toArray();
        }
    }
    return {};
}

QJsonArray Actions::parsePrototype(uint32_t prototype) {
    PlayItemIterator pit(false);
    if (m_prototypes.contains(prototype)) {
        QJsonObject root = m_prototypes.value(prototype)->toJson({}, pit);
        if (root.contains("children")) {
            return root.value("children").toArray();
        }
    }
    return {};
}

QAbstractItemModel* Actions::prototypeModel(uint32_t prototype) {
    if (m_prototypes.contains(prototype)) {
        return m_prototypes.value(prototype);
    }
    return nullptr;
}


bool Actions::execAction(QString action_id, PlayItemIterator context) {
    DBAction *action = getAction(action_id);
    if (action) {
        action->apply(context);
        return true;
    }
    return false;
}

DBAction* Actions::getAction(QString action_id) {
    DBAction *action = nullptr;
    for (ActionOwner *owner : m_action_owners) {
        action = owner->getAction(action_id);
        if (action) {
            break;
        }
    }
    return action;
}

QStringList Actions::getActions() {
    QStringList actions;
    for (ActionOwner *owner : m_action_owners) {
        QList<DBAction*> owner_actions = owner->getActions();
        for (DBAction* action : owner_actions) {
            actions.append(action->action_id);
        }
    }
    return actions;
}

QString Actions::getActionTitle(QString action_id) {
    DBAction *action = getAction(action_id);
    if (action) {
        return action->title;
    }
    return QString();
}

QHash<QString, QVariant> Actions::getActionContext(QString action_id, PlayItemIterator &context) {
    DBAction *action = getAction(action_id);
    if (action) {
        return action->contextualize(context);
    }
    return QHash<QString, QVariant>{};
}






//QJsonArray Actions::getTreeForLocation(DBAction::ActionLocations location, PlayItemIterator &pit) {
//    QJsonObject a = actionModelToJson(m_actions[locToNum(location)], QModelIndex(), pit);
//    qDebug() << "AAA" << a;
//    return a.value("children").toArray();
//}

//QAbstractItemModel* Actions::getModelForLocation(DBAction::ActionLocations location) {
//    return m_actions[locToNum(location)];
//}


QString Actions::getDefaultConfig(DBAction::ActionLocations location) {
    switch (location) {
        case DBAction::ACTION_LOC_MENUBAR:
        return "[{\"action\":\"q_open_files\",\"properties\":{\"separator_after\":true}},{\"action\":\"q_add_files\"},{\"action\":\"q_add_folders\"},{\"action\":\"q_add_location\"},{\"action\":\"cd_add\"},{\"action\":\"q_new_playlist\",\"properties\":{\"separator_before\":true}},{\"action\":\"q_load_playlist\"},{\"action\":\"q_save_playlist\"},{\"action\":\"q_quit\",\"properties\":{\"separator_before\":true,\"last\":true}},{\"action\":\"q_clear\"},{\"action\":\"q_design_mode\"},{\"action\":\"q_shuffle_off\"},{\"action\":\"q_repeat_all\"},{\"action\":\"q_scroll_follows_playback\"},{\"action\":\"q_cursor_follows_playback\"},{\"action\":\"q_stop_after_current_track\"},{\"action\":\"q_stop_after_current_album\"},{\"action\":\"q_jump_to_current_track\"},{\"action\":\"q_help\"},{\"action\":\"q_changelog\"},{\"action\":\"q_gplv2\"},{\"action\":\"q_lgplv21\"},{\"action\":\"q_about\"},{\"action\":\"q_about_qt\"},{\"action\":\"q_translators\"}]";
        break;
        case DBAction::ACTION_LOC_TRACK_CONTEXT:
            return "[{\"action\":\"add_to_front_of_playback_queue\",\"properties\":{\"extract\":true}},{\"action\":\"add_to_playback_queue\",\"properties\":{\"extract\":true}},{\"action\":\"remove_from_playback_queue\",\"properties\":{\"extract\":true,\"separator_after\":true}},{\"action\":\"q_cut\"},{\"action\":\"q_copy\"},{\"action\":\"q_paste\",\"properties\":{\"separator_after\":true}},{\"action\":\"q_track_properties\",\"properties\":{\"last\":true}}]";
            break;
    }
    return {};
}

QHash<QString, QVariant> jsonPropsConvert(QJsonObject obj) {
    QHash<QString, QVariant> out;
    for (QString key : obj.keys()) {
        out.insert(key, obj.value(key).toVariant());
    }
    return out;
}


//void Actions::rebuildMenu(DBAction::ActionLocations location) {

//    ActionsModel *replacement = new ActionsModel((QObject*)this, deadbeef, location);

//    QList<DBAction *> import;

//    // default actions
//    for (DBAction *action : default_actions->m_actions) {
//        if (action->locations & location) {
//            import.append(action);
//        }
//    }

//    // plugin actions
//    {
//        DB_plugin_t **pluglist = DBAPI->plug_get_list();
//        int i = 0;
//        while (pluglist[i]) {
//            if (pluglist[i]->get_actions) {
//                DB_plugin_action_t *itr = pluglist[i]->get_actions(nullptr); // kinda off implementation :(
//                while (itr) {
//                    if (m_actions_hash.contains(itr->name)) {
//                        if (location & m_actions_hash.value(itr->name)->locations) {
//                            import.append(m_actions_hash.value(itr->name));
//                        }
//                    }
//                    else {
//                        DBActionImported *a = new DBActionImported(deadbeef, itr);
//                        bool imported = false;
//                        a->setParent(this);

//                        if (location & a->locations) {
//                            import.append(a);
//                            imported = true;
//                        }
//                        else {
//                            delete a;
//                        }
//                    }
//                    itr = itr->next;
//                }
//            }
//            i++;
//        }
//    }

//    // cache names
//    for (DBAction *a : qAsConst(import)) {
//        m_actions_hash.insert(a->action_id, a);
//    }

//    // sort and insert to model
//    QJsonArray arr = getDefaultConfig(location);

//    QList<DBAction *> last_actions;
//    QList<DBAction *> import_sorted;
//    QSet<DBAction *> import_ignore;

//    for (int i = 0; i < arr.size(); i++) {
//        QJsonObject obj = arr.at(i).toObject();
//        QString action_id_search = obj.value("action").toString();
//        QJsonObject properties_json = obj.value("properties").toObject();
//        QHash<QString, QVariant> properties = jsonPropsConvert(properties_json);
//        DBAction *action = m_actions_hash.value(action_id_search);

//        if (!action) {ngle>> {

//                json ActionTree_JSON2 <<action>> {
//                    "type": "\"action\"",
//                             "text" : "//action title//",
//                                      "id" : "//action id name//",
//                                             "properties" : "//key→valu
//            qDebug() << "ACTION" << action_id_search << "not found, skipping";
//            continue;
//        }


//        if (properties_json.contains("hide") && properties_json.value("hide").toBool()) {
//            import_ignore.insert(action);
//            continue;
//        }


//        action->properties_const.insert(properties);

//        if (properties_json.contains("last") && properties_json.value("last").toBool()) {
//            last_actions.append(action);
//            continue;
//        }

//        if (properties_json.contains("extract") && properties_json.value("extract").toBool()) {
//            action->path = QStringList(action->path.last());
//        }

//        import_sorted.append(action);
//    }

//    for (DBAction *action : import) {
//        if (!import_sorted.contains(action) && !last_actions.contains(action) && !import_ignore.contains(action)) {
//            import_sorted.append(action);
//        }
//    }

//    import_sorted.append(last_actions);

//    replacement->insertActions(import_sorted);

//    ActionsModel *old = m_actions[locToNum(location)];
//    m_actions[locToNum(location)] = replacement;
//    if (old)
//        delete old;
//}

//static void printAction(DBActionImported *action) {
//    qDebug() << "Action:" << action->action_id;
//    // action accepts;
//    QList<int> accepts = {DBAction::ACTION_ARG_NONE, DBAction::ACTION_ARG_TRACK, DBAction::ACTION_ARG_TRACKS, DBAction::ACTION_ARG_PLAYLIST};
//    QList<QString> accepts_str = {"ACTION_ARG_NONE", "ACTION_ARG_TRACK", "ACTION_ARG_TRACKS", "ACTION_ARG_PLAYLIST"};
//    QStringList accepts_list;
//    for (int i = 0; i < accepts.size(); i++) {
//        if (accepts[i] & action->accepts) {
//            accepts_list.append(accepts_str[i]);
//        }
//    }
//    QList<int> locations = {DBAction::ACTION_LOC_HOTKEY, DBAction::ACTION_LOC_TRACK_CONTEXT, DBAction::ACTION_LOC_PLAYLIST_CONTEXT, DBAction::ACTION_LOC_MENUBAR};
//    QList<QString> locations_str = {"ACTION_LOC_HOTKEY", "ACTION_LOC_TRACK_CONTEXT", "ACTION_LOC_PLAYLIST_CONTEXT", "ACTION_LOC_MENUBAR"};
//    QStringList locations_list;
//    for (int i = 0; i < locations.size(); i++) {
//        if (locations[i] & action->locations) {
//            locations_list.append(locations_str[i]);
//        }
//    }
//    qDebug() << "Accepts:" << accepts_list.join(" | ");
//    qDebug() << "Locations:" << locations_list.join(" | ");
//}



// JSON PROPERTIES

// separator_after: true/false
// separator_before: true/false
// last: true/false (place at the list)
// hide: true/false (do not show)
// extract: true/false (remove action from submenu(s))
// checkable: true/false
// checked: true/false
// exclusive_group: string (exclusive group)
// text: string (override title)

// ???config???
// ???config_default???
