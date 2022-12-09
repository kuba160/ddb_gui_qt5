#include "Actions.h"

#include "models/ActionsModel.h"
#include <QJsonObject>

#define DBAPI (this->deadbeef)

DB_functions_t* ActionContext::deadbeef;

ActionContext::ActionContext(ddb_playlist_t* plt) {
    this->plt = plt;
    deadbeef->plt_ref(plt);
    this->mode = DBAction::ACTION_PLAYLIST;
}

ActionContext::ActionContext(QList<DB_playItem_t*> tracks) {
    this->mode = DBAction::ACTION_SELECTION;
    this->it_list = tracks;
    for (DB_playItem_t *it : it_list) {
        deadbeef->pl_item_ref(it);
    }
}

ActionContext::ActionContext(DB_playItem_t* track) {
    this->mode = DBAction::ACTION_SELECTION;
    this->it_list = QList<DB_playItem_t*>();
    this->it_list.append(track);
    deadbeef->pl_item_ref(track);
}

ActionContext::ActionContext(bool nowplaying) {
    if (nowplaying) {
        this->mode = DBAction::ACTION_NOWPLAYING;
    }
    else {
        this->mode = DBAction::ACTION_MAIN;
    }
}

ActionContext::~ActionContext() {
    if (mode == DBAction::ACTION_PLAYLIST) {
        deadbeef->plt_unref(plt);
    }
    else if (mode == DBAction::ACTION_SELECTION) {
        for (DB_playItem_t *it : it_list) {
            deadbeef->pl_item_unref(it);
        }
    }
}

void ActionContext::executeForAction(DBAction* action) {

    if (mode == DBAction::ACTION_PLAYLIST) {
        deadbeef->action_set_playlist(plt);
        action->actionExecute(DBAction::ACTION_PLAYLIST);
    }
    else if (mode == DBAction::ACTION_SELECTION) {
        if (action->action_id == "add_to_playback_queue") {
            for (DB_playItem_t *it : it_list) {
                deadbeef->playqueue_push(it);
            }
            deadbeef->sendmessage(DB_EV_PLAYLIST_REFRESH, 0, 0, 0);
            return;
        }
        else if (action->action_id == "remove_from_playback_queue") {
            for (DB_playItem_t *it : it_list) {
                deadbeef->playqueue_remove(it);
            }
            deadbeef->sendmessage(DB_EV_PLAYLIST_REFRESH, 0, 0, 0);
            return;
        }
        /*
        ddb_playlist_t *plt_tmp = deadbeef->plt_alloc("_dbapi_tmp");
        if (plt_tmp) {
            deadbeef->plt_ref(plt_tmp);
            deadbeef->plt_ref(plt_tmp);
            DB_playItem_t *it_last = nullptr;
            for (DB_playItem_t *it : it_list) {
                deadbeef->plt_insert_item(plt_tmp, it_last, it);
                deadbeef->pl_item_ref(it);
                it_last = it;
            }
            deadbeef->action_set_playlist(plt_tmp);
            action->actionExecute(DBAction::ACTION_PLAYLIST);
            //deadbeef->action_set_playlist(nullptr);
            //deadbeef->plt_unref(plt_tmp);
            //deadbeef->plt_free(plt_tmp);
        }*/
    }
    else {
        action->actionExecute(mode);
    }
}

Actions::Actions(QObject *parent, DB_functions_t *Api)
    : QObject{parent} {
    deadbeef = Api;
    ActionContext::deadbeef = Api;

    m_actions = new ActionsModel(this, Api);
    m_actions_menu = new ActionsModel(this, Api, DB_ACTION_ADD_MENU);
    m_actions_track= new ActionsModel(this, Api, DB_ACTION_MULTIPLE_TRACKS);

    QList<DBAction *> import_all;
    QList<DBAction *> import_menu;
    QList<DBAction *> import_track;
    QList<DBAction *> import_playlist;
    {
        DB_plugin_t **pluglist = DBAPI->plug_get_list();
        int i = 0;
        while (pluglist[i]) {
            if (pluglist[i]->get_actions) {
                DB_plugin_action_t *itr = pluglist[i]->get_actions(nullptr); // kinda off implementation :(
                while (itr) {
                    DBActionImported *a = new DBActionImported(deadbeef, itr);
                    a->setParent(this);
                    import_all.append(a);
                    if (itr->flags & DB_ACTION_COMMON &&
                        itr->flags & DB_ACTION_ADD_MENU) {
                        import_menu.append(a);
                    }
                    if (itr->flags & (DB_ACTION_MULTIPLE_TRACKS) &&
                        !(itr->flags & DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST)) {
                        if (!trackMenuExcludeAction(a)) {
                            import_track.append(a);
                        }
                    }
                    if (itr->flags & DB_ACTION_PLAYLIST) {
                        import_playlist.append(a);
                    }
                    itr = itr->next;
                }
            }
            i++;
        }
    }
    m_actions->insertActions(import_all);
    m_actions_menu->insertActions(import_menu);
    m_actions_track->insertActions(import_track);
    //m_actions->insertActions(import_all); playlist todo

    for (DBAction *a : qAsConst(import_all)) {
        m_actions_hash.insert(a->action_id, a);
    }
}

bool Actions::registerActionsBuilder(QString name, actionsBuilderConstructor builder) {
    if (!m_actions_builders.contains(name)) {
        m_actions_builders.insert(name, builder);
        return true;
    }
    return false;
}

bool Actions::execAction(QString action, ActionContext context) {
    if (m_actions_hash.contains(action)) {
        context.executeForAction(m_actions_hash.value(action));
        return true;
    }
    return false;
}

QAbstractItemModel* Actions::getActionsModel() const {
    return m_actions;
}

QAbstractItemModel* Actions::getActionsMenuModel() const {
    return m_actions_menu;
}

QString Actions::getActionTitle(QString action_id) {
    if (m_actions_hash.contains(action_id)) {
        return m_actions_hash.value(action_id)->title;
    }
    return QString();
}

QVariant Actions::buildActionMenu(QObject *parent, QString name, ActionContext *context) {
    QModelIndex idx{};
    if (m_actions_builders.contains(name)) {
        if (!context) {
            context = new ActionContext(false);
        }
        ActionsBuilder *menu_builder = m_actions_builders.value(name)(parent,context);
        context->setParent(menu_builder->returnValue().value<QObject*>());
        if (menu_builder) {
            buildActionMenuIter(menu_builder, idx);
            return menu_builder->returnValue();
        }
    }
    qDebug() << "Failed to build menu" << name;
    return {};
}

void Actions::buildActionMenuIter(ActionsBuilder *menu, QModelIndex &parent) {
    QAbstractItemModel *model = m_actions_menu;
    for (int i = 0; i < model->rowCount(parent); i++) {
        QModelIndex idx = model->index(i,0,parent);
        if (!idx.isValid()) {
            continue;
        }
        QString title = model->data(idx).toString();
        if (model->hasChildren(idx)) {
            // submenu
            ActionsBuilder *child = menu->createSubMenu(title);
            buildActionMenuIter(child, idx);
        }
        else {
            DBAction *action =  model->data(idx, ActionsModel::ACTION_NODE).value<DBAction *>();
            menu->insertAction(action);
        }
    }
}

QVariant Actions::buildTrackMenu(QObject *parent, QString name, ActionContext *context) {
    QModelIndex idx{};
    if (m_actions_builders.contains(name)) {
        ActionsBuilder *menu_builder = m_actions_builders.value(name)(parent, context);
        if (menu_builder) {
            buildTrackMenuIter(menu_builder, idx);
            return menu_builder->returnValue();
        }
    }
    qDebug() << "Failed to build track menu" << name;
    return {};
}

void Actions::buildTrackMenuIter(ActionsBuilder *menu, QModelIndex &parent) {
    QAbstractItemModel *model = m_actions_track;
    for (int i = 0; i < model->rowCount(parent); i++) {
        QModelIndex idx = model->index(i,0,parent);
        if (!idx.isValid()) {
            continue;
        }
        QString title = model->data(idx).toString();
        if (model->hasChildren(idx)) {
            if (title == "Playback") {
                buildTrackMenuIter(menu, idx);
            }
            else {
                // submenu
                ActionsBuilder *child = menu->createSubMenu(title);
                buildTrackMenuIter(child, idx);
            }
        }
        else {
            DBAction *action =  model->data(idx, ActionsModel::ACTION_NODE).value<DBAction *>();
            menu->insertAction(action);
        }
    }
}

bool Actions::trackMenuExcludeAction(DBAction *action) {
    if (action->action_id == "add_to_front_of_playback_queue") {
        return true;
    }
    if (action->action_id == "duplicate_playlist") {
        return true;
    }
    return false;
}

DBActionImported::DBActionImported(DB_functions_t* deadbeef, DB_plugin_action_t *ptr) : DBAction(nullptr) {
    this->deadbeef = deadbeef;
    setObjectName(ptr->name);
    plug_action = ptr;
    callback2 = ptr->callback2;


    static QRegularExpression re("\\/(?<!\\\\\\/)"); // regex go brrrrr
    QStringList strlist = QString(ptr->title).split(re);
    title = strlist.last();
    title = title.replace("\\/", "/");
    path = ptr->title;
    action_id = ptr->name;
    icon = DBActionImported::iconOnAction(action_id);
    flags = ptr->flags;
    enabled = true;
}

QString DBActionImported::iconOnAction(const QString action) {
    static QHash<QString,QString> icon_map = {
        {"reload_metadata","view-refresh"},
        {"lfm_lookup","edit-find"},
        {"track_properties","document-properties"},
        {"add_to_playback_queue","list-add"},
        {"remove_from_playback_queue","list-remove"},
        {"cut","edit-cut"},
        {"copy","edit-copy"},
        {"paste","edit-paste"},
        {"delete","edit-delete"},
        {"playlist_rename","edit-rename"},
        {"playlist_delete","edit-delete-remove"},
        {"playlist_add","media-playlist-append"},
        {"playlist_duplicate","document-duplicate"},
        {"skip_to_prev_genre", "view-media-genre"},
        {"skip_to_prev_composer", "view-media-similarartists"},
        {"skip_to_prev_artist", "view-media-artist"},
        {"skip_to_prev_album", "view-media-album-cover"},
        {"skip_to_next_genre", "view-media-genre"},
        {"skip_to_next_composer", "view-media-similarartists"},
        {"skip_to_next_artist", "view-media-artist"},
        {"skip_to_next_album", "view-media-album-cover"},
        {"cd_add", "gtk-cdrom"},
    };
    if (icon_map.contains(action)) {
        return icon_map.value(action);
    }
    return QString();
}

void DBActionImported::actionExecute(ActionExecuteMode role) const {
    if (callback2) {
        switch (role) {
            case DBAction::ACTION_MAIN:
            case DBAction::ACTION_PLAYLIST:

                // todo
                //break;
            case DBAction::ACTION_SELECTION:
                // todo
                break;
            case DBAction::ACTION_NOWPLAYING:
                callback2(plug_action, (ddb_action_context_t) role);
                break;
        }
    }
}


/*
ActionsJsonBuilder::ActionsJsonBuilder(QObject *parent) : QObject(parent) {

}

ActionsBuilder * ActionsJsonBuilder::createSubMenu(QString &title) {
    ActionsJsonBuilder *sub = new ActionsJsonBuilder(this);
    submenus.append(sub->json_obj);
    return sub;
}

void ActionsJsonBuilder::insertAction(DBAction *action) {
    actions.append(action->action_id);
}

QByteArray ActionsJsonBuilder::toJson(QJsonDocument::JsonFormat format) {
    json_obj.insert("submenus", submenus);
    json_obj.insert("actions", actions);
    QJsonDocument doc(json_obj);
    return doc.toJson(format);
}*/
