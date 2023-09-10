#include "ActionsDeadbeef.h"



DBActionImported::DBActionImported(DB_functions_t* deadbeef, DB_plugin_action_t *ptr) : DBAction(nullptr) {
    this->deadbeef = deadbeef;
    setObjectName(ptr->name);
    action_original = ptr;

    static QRegularExpression re("\\/(?<!\\\\\\/)"); // regex go brrrrr
    path = QString(ptr->title).split(re);
    title = path.last();
    title = title.replace("\\/", "/");

    action_id = ptr->name;

    accepts = actionAcceptsFromFlags(ptr->flags);
    locations = actionLocationsFromFlags(ptr->flags);

}


QString DBActionImported::iconOnAction(const QString action) {
    static QHash<QString,QString> icon_map = {
                                               {"reload_metadata","view-refresh"},
                                               {"lfm_lookup","edit-find"},
                                               {"track_properties","document-properties"},
                                               {"add_to_playback_queue","list-add"},
                                               {"add_to_front_of_playback_queue", "go-first"},
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

uint16_t DBActionImported::actionAcceptsFromFlags(uint32_t flags) {
    uint16_t out = 0;
    if (flags & DB_ACTION_COMMON) {
        out = out | ACTION_ARG_NONE;
    }
    if (flags & DB_ACTION_SINGLE_TRACK) {
        out = out | ACTION_ARG_TRACK;
    }
    if (flags & DB_ACTION_MULTIPLE_TRACKS) {
        out = out | ACTION_ARG_TRACKS;
    }
    if (flags & DB_ACTION_PLAYLIST) {
        out = out | ACTION_ARG_PLAYLIST;
    }
    return out;
}
uint16_t DBActionImported::actionLocationsFromFlags(uint32_t flags) {
    uint16_t out = 0;
    if (!(flags & DB_ACTION_DISABLED)) {
        out = out | ACTION_LOC_HOTKEY;
    }
    if (flags & DB_ACTION_ADD_MENU || flags & DB_ACTION_MULTIPLE_TRACKS) {
        if (flags & DB_ACTION_COMMON) {
            out = out | ACTION_LOC_MENUBAR;
        }
        if (flags & DB_ACTION_SINGLE_TRACK) {
            out = out | ACTION_LOC_TRACK_CONTEXT;
        }
        if (flags & DB_ACTION_MULTIPLE_TRACKS) {
            //out = out | ACTION_LOC_TRACK_CONTEXT;
            if (!(flags & DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST)) {
                out = out | ACTION_LOC_TRACK_CONTEXT;
                out = out | ACTION_LOC_PLAYLIST_CONTEXT;
            }
        }
        if (flags & DB_ACTION_PLAYLIST) {
            out = out | ACTION_LOC_PLAYLIST_CONTEXT;
        }
    }
    return out;
}

QHash<QString, QVariant> DBActionImported::contextualize(PlayItemIterator &context) const {
    QHash<QString,QVariant> ret;
    ret.insert(this->properties_const);

    // icon lookup
    QString icon = iconOnAction(action_id);
    if (!icon.isNull()) {
        ret.insert("icon", icon);
    }

    // disable removing from queue if track is not queued
    if (action_id == "remove_from_playback_queue") {
        DB_playItem_t *it = nullptr;
        bool inQueue = false;
        if (deadbeef->playqueue_get_count()) {
            while ((it = context.getNextIter())) {
                if (deadbeef->playqueue_test(it) != -1) {
                    inQueue = true;
                    break;
                }
            }
            context.resetIter();
        }
        if (!inQueue) {
            ret.insert("enabled", false);
        }
    }
    else {
        // intention: do not allow actions that require track(s) to executed if iterator is empty
        // disabled: breaks skip to
//        DB_playItem_t *it = context.getNextIter();
//        if (!it) {
//            ret.insert("enabled", false);
//        }
//        context.resetIter();

    }
    return ret;
}

bool DBActionImported::apply(PlayItemIterator &context) {
    // legacy apply works by copying tracks to a new playlist
    // doesn't work that well with queue, which means it has to be done manually
    if (action_id == "add_to_playback_queue") {
        DB_playItem_t *it = nullptr;
        while ((it = context.getNextIter())) {
            deadbeef->playqueue_push(it);
        }
        deadbeef->sendmessage(DB_EV_PLAYLIST_REFRESH, 0, 0, 0);
    }
    else if (action_id == "add_to_front_of_playback_queue") {
        DB_playItem_t *it = nullptr;
        int i = 0;
        while ((it = context.getNextIter())) {
            deadbeef->playqueue_insert_at(i, it);
        }
        deadbeef->sendmessage(DB_EV_PLAYLIST_REFRESH, 0, 0, 0);
    }
    else if (action_id == "remove_from_playback_queue") {
        DB_playItem_t *it = nullptr;
        while ((it = context.getNextIter())) {
            deadbeef->playqueue_remove(it);
        }
        deadbeef->sendmessage(DB_EV_PLAYLIST_REFRESH, 0, 0, 0);
    }
    else {
        context.apply_legacy(action_original);
    }
    emit actionApplied(context);
    return true;
};


ActionsDeadbeef::ActionsDeadbeef(QObject *parent, DBApi *Api)
    : ActionOwner{parent} {

    DB_plugin_t **pluglist = Api->deadbeef->plug_get_list();
    int i = 0;
    while (pluglist[i]) {
        if (pluglist[i]->get_actions) {
            DB_plugin_action_t *itr = pluglist[i]->get_actions(nullptr); // kinda off implementation :(
            while (itr) {
                DBActionImported *a = new DBActionImported(Api->deadbeef, itr);
                m_actions.append(a);
                a->setParent(this);
                itr = itr->next;
            }    
        }
        i++;
    }
}



ActionsDeadbeef::~ActionsDeadbeef() {
    while(!m_actions.empty()) {
        DBAction *action = m_actions.takeFirst();
        emit actionAboutToBeDeleted(action->action_id);
        delete action;
    }
}


QList<DBAction*> ActionsDeadbeef::getActions() {
    return m_actions;
}
DBAction* ActionsDeadbeef::getAction(QString action_id) {
    for (DBAction *action : m_actions) {
        if (action->action_id == action_id) {
            return action;
        }
    }
    return nullptr;
}
