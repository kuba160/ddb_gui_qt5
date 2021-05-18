#include "ActionManager.h"
#include "DBApi.h"
#include <QMenu>
#include <QGuiApplication>
#include "MainWindow.h"
#include "DefaultActions.h"

ActionItem::ActionItem(DB_plugin_action_t *ptr) : QAction(){
    setObjectName(ptr->name);
    setText(tr(ptr->title));
    setProperty("Title",QString(ptr->title));
    setProperty("Flags",QVariant(ptr->flags).toULongLong());

    plug_action = ptr;
    callback2 = ptr->callback2;

    connect(this, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
    setDefaultIcon(this);

    QRegularExpression re("\\/(?<!\\\\\\/)"); // regex go brrrrr
    QStringList strlist = QString(ptr->title).split(re);
    if (strlist.length() > 1) {
        nested = true;
    }
}

bool ActionItem::isNested() {
    return nested;
}

QString ActionItem::iconOnAction(QString action) {
    static QHash<QString,QString> icon_map;
    if (icon_map.isEmpty()) {
        icon_map.insert("reload_metadata","view-refresh");
        icon_map.insert("lfm_lookup","edit-find");
        icon_map.insert("track_properties","document-properties");
        icon_map.insert("add_to_playback_queue","list-add");
        icon_map.insert("remove_from_playback_queue","list-remove");
        icon_map.insert("cut","edit-cut");
        icon_map.insert("copy","edit-copy");
        icon_map.insert("paste","edit-paste");
        icon_map.insert("delete","edit-delete");
        icon_map.insert("playlist_rename","edit-rename");
        icon_map.insert("playlist_delete","edit-delete-remove");
        icon_map.insert("playlist_add","media-playlist-append");
        icon_map.insert("playlist_duplicate","document-duplicate");


    }
    if (icon_map.contains(action)) {
        return icon_map.value(action);
    }
    return QString();
}

void ActionItem::setDefaultIcon(QAction *action) {
    QString icon_name = iconOnAction(action->objectName());
    if (!icon_name.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon_name));
    }
}

void ActionItem::onActionTriggered() {
    DB_plugin_action_t *orig_action = static_cast<DB_plugin_action_t *>(property("DBACTION").value<void *>());
    // TODO better support for flags
    if (property("Flags").toULongLong() & DB_ACTION_COMMON) {
        callback2(orig_action, DDB_ACTION_CTX_MAIN);
    }
    else {
        callback2(orig_action, DDB_ACTION_CTX_SELECTION);
    }

}

ActionManager::ActionManager (QObject *parent, DBApi *Api) : QObject(parent) {
    api = Api;
    DefaultActions *da = new DefaultActions(Api);

    mainMenuBar = da->getDefaultMenuBar();
    loadActions();
    fillMenuBar();
}

void ActionManager::fillMenuBar() {
    foreach(ActionItem *it, actions) {
        uint32_t flags = it->property("Flags").toULongLong();
        if (it->isNested() && ((flags & (DB_ACTION_COMMON | DB_ACTION_ADD_MENU)) == (DB_ACTION_COMMON | DB_ACTION_ADD_MENU))) {
            // iterate for separators in title
            QRegularExpression re("\\/(?<!\\\\\\/)");
            QStringList strlist = QString(it->property("Title").toString()).split(re);
            int i;
            QMenu *currmenu = nullptr;
            for (i = 0; i < strlist.length() - 1; i++) {
                // iterate through current children to find same dir
                // if not, create new one
                QMenu *futuremenu;
                if (!currmenu) {
                    // find root menu
                    // normally called menuNAME...
                    futuremenu = mainMenuBar->findChild<QMenu *>(QString("menu%1") .arg(strlist[i]), Qt::FindDirectChildrenOnly);
                    // ... but File menu gets translated to "menu"? ...
                    if (!futuremenu && strlist[i] == "File") {
                        futuremenu = mainMenuBar->findChild<QMenu *>(QString("menu"), Qt::FindDirectChildrenOnly);
                    }
                    // ... and Help menu gets translated to "Help"?
                    if (!futuremenu) {
                        futuremenu = mainMenuBar->findChild<QMenu *>(strlist[i], Qt::FindDirectChildrenOnly);
                    }
                    // very weird behaviour from qt...
                }
                else {
                    futuremenu = currmenu->findChild<QMenu *>(strlist[i].toUtf8());
                }
                if (futuremenu) {
                    currmenu = futuremenu;
                    continue;
                }
                else {
                    // create
                    if (!currmenu) {
                        // in menubar
                        futuremenu = mainMenuBar->addMenu(tr(strlist[i].toUtf8()));
                        futuremenu->setObjectName(QString("menu") + strlist[i]);
                    }
                    else {
                        // in current menu
                        futuremenu = currmenu->addMenu(tr(strlist[i].toUtf8()));
                        futuremenu->setObjectName(QString("menu") + strlist[i]);
                    }
                    futuremenu->setObjectName(strlist[i]);
                    currmenu = futuremenu;
                    continue;
                }
            }
            if (currmenu) {
                currmenu->addAction(it);
                it->setText(tr(strlist[i].toUtf8()));
            }
        }
    }
}

ActionManager::~ActionManager() {
    int i;
    for (i = 0; i < actions.length(); i++) {
        delete actions[i];
    }
}

void ActionManager::loadActions() {
    DB_plugin_t **pluglist = DBAPI->plug_get_list();
    int i = 0;
    while (pluglist[i]) {
        if (pluglist[i]->get_actions) {
            // append all actions
            DB_plugin_action_t *itr = pluglist[i]->get_actions(nullptr); // kinda off implementation :(
            while(itr) {
                actions.append(new ActionItem(itr));
                itr = itr->next;
            }
        }
        i++;
    }
}

int ActionManager::menuActionsAvailable(QObject *obj) {
    int ag = ActionsEmpty;

    QVariant wactions = obj->property("Actions");
    QActionGroup *actions_group = reinterpret_cast<QActionGroup *>(wactions.value<quintptr>());
    if (wactions.isValid() && actions_group) {
        QList<QAction*> l = actions_group->actions();

        while (l.length()) {
            QAction *a = l[0];
            if(a->objectName() == "add_to_playback_queue" ||
               a->objectName() == "remove_from_playback_queue") {
                l.removeFirst();
                ag = ag | ActionsPlayback;
            }
            else if (a->objectName() == "cut" ||
                    (a->objectName() == "copy") ||
                    (a->objectName() == "paste")) {
                l.removeFirst();
                ag = ag | ActionsClipboard;
                continue;
            }
            else if (a->objectName() == "delete") {
                l.removeFirst();
                ag = ag | ActionsDelete;
            }
            else if (a->objectName() == "track_properties") {
                l.removeFirst();
                ag = ag | ActionsTrackProp;
            }
            else {
                l.removeFirst();
                ag = ag | ActionsCustom;
            }
        }
        if (!obj->property("pluginActionsDisabled").toBool()) {
            ag = ag |ActionsPlugins;
        }
    }
    return ag;
}

void ActionManager::insertActionWithName(QMenu *menu, QList<QAction*> *l, QString name) {
    for (int i = 0; i < l->length(); i++) {
        QAction *a = l->at(i);
        if(a->objectName() == name) {
            menu->addAction(a);
            ActionItem::setDefaultIcon(a);
            l->removeOne(a);
            break;
        }
    }
}

QList<QAction *> ActionManager::defaultPlaylistActions() {
    static QList<QAction *> actions;
    if (!actions.length()) {
        actions.append(new QAction(tr("Rename Playlist")));
        actions.last()->setObjectName("playlist_rename");
        connect(actions.last(), SIGNAL(triggered()), this, SLOT(onChangePlaylistName()));
        ActionItem::setDefaultIcon(actions.last());

        actions.append(new QAction(tr("Remove Playlist")));
        actions.last()->setObjectName("playlist_delete");
        connect(actions.last(), SIGNAL(triggered()), this, SLOT(onDeletePlaylist()));
        ActionItem::setDefaultIcon(actions.last());

        actions.append(new QAction(tr("Add New Playlist")));
        actions.last()->setObjectName("playlist_add");
        connect(actions.last(), SIGNAL(triggered()), this, SLOT(onAddNewPlaylist()));
        ActionItem::setDefaultIcon(actions.last());
    }
    return actions;
}

void ActionManager::playItemContextMenu(QWidget *parent, QPoint p) {
    QMenu *ctxMenu = new QMenu(parent);

    // Sort actions and format them as followed:
    QVariant wactions = parent->property("Actions");
    QActionGroup *actions_group = reinterpret_cast<QActionGroup *>(wactions.value<quintptr>());
    if (wactions.isValid() && actions_group) {
        int acts = menuActionsAvailable(parent) + ActionsTrackProp;
        QList<QAction*> l = actions_group->actions();

        if (acts & ActionsPlayback) {
            insertActionWithName(ctxMenu, &l, "add_to_playback_queue");
            insertActionWithName(ctxMenu, &l, "remove_from_playback_queue");
        }
        if (acts & ActionsClipboard) {
            ctxMenu->addSeparator();
            insertActionWithName(ctxMenu, &l, "cut");
            ctxMenu->actions().last()->setShortcut(QKeySequence::Cut);
            insertActionWithName(ctxMenu, &l, "copy");
            ctxMenu->actions().last()->setShortcut(QKeySequence::Copy);
            insertActionWithName(ctxMenu, &l, "paste");
            ctxMenu->actions().last()->setShortcut(QKeySequence::Paste);
        }
        if (acts & ActionsDelete) {
            ctxMenu->addSeparator();
            insertActionWithName(ctxMenu, &l, "delete");
            ctxMenu->actions().last()->setShortcut(QKeySequence::Delete);
        }
        if (acts & ActionsPlugins) {
            ctxMenu->addSeparator();
            foreach (ActionItem *ait, actions) {
                uint32_t flags = ait->property("Flags").toULongLong();
                if (((flags & (DB_ACTION_SINGLE_TRACK)) || (flags & DB_ACTION_MULTIPLE_TRACKS)) &&
                    !(flags & DB_ACTION_COMMON) && !(flags & DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST)) {
                    if (ait->objectName() == "add_to_playback_queue" || ait->objectName() == "remove_from_playback_queue") {
                        continue;
                    }
                    ctxMenu->addAction(ait);
                    if (!parent->property("playItemsSelected").toInt()) {
                        ait->setEnabled(false);
                    }
                    else {
                        ait->setEnabled(true);
                    }
                }
            }
        }
        if (acts & ActionsCustom) {
            ctxMenu->addSeparator();
            ctxMenu->addActions(l);
        }
        if (acts & ActionsTrackProp) {
            // TODO
            ctxMenu->addSeparator();
            QAction *track_properties = ctxMenu->addAction(tr("Track Properties"));
            track_properties->setObjectName("track_properties");
            ActionItem::setDefaultIcon(track_properties);
            track_properties->setEnabled(false);
        }
    }
    ctxMenu->popup(parent->mapToGlobal(QPoint(0,0)) + p);

    // Add to queue
    // Delete from queue
    // ---
    // Reload metadata
    // Cut
    // Copy
    // Paste
    // ---
    // Delete
    // ---
    // (Plugins I suppose)
    // ReplayGain (menu)
    // -/ Scan gain separately if not scanned
    // -/ Scan gain separately
    // -/ Scan Selection as album
    // -/ Scan Selection as albums (after labels)
    // -/ Delete ReplayGain information
    // Refresh cover
    // Convert
    // Search on Last.fm
    // ---
    // Track properties
    return;
}

void ActionManager::playlistContextMenu(QWidget *parent, QPoint p, int plt) {
    QMenu *ctxMenu = new QMenu(parent);

    playlist_number = plt;
    ctxMenu->addActions(defaultPlaylistActions());

    // Sort actions and format them as followed:
    QVariant wactions = parent->property("Actions");
    if (wactions.isValid()) {
        int acts = menuActionsAvailable(parent) + ActionsTrackProp;
        QActionGroup *actions_group = reinterpret_cast<QActionGroup *>(wactions.toUInt());
        QList<QAction*> l = actions_group->actions();

        if (acts & ActionsPlugins) {
            ctxMenu->addSeparator();
            foreach (ActionItem *ait, actions) {
                uint32_t flags = ait->property("Flags").toULongLong();
                if (((flags & (DB_ACTION_SINGLE_TRACK)) || (flags & DB_ACTION_MULTIPLE_TRACKS)) &&
                    !(flags & DB_ACTION_COMMON) && !(flags & DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST)) {
                    if (ait->objectName() == "add_to_playback_queue" || ait->objectName() == "remove_from_playback_queue") {
                        continue;
                    }
                    ctxMenu->addAction(ait);
                    if (!parent->property("playItemsSelected").toInt()) {
                        ait->setEnabled(false);
                    }
                    else {
                        ait->setEnabled(true);
                    }
                }
            }
        }
        if (acts & ActionsCustom) {
            ctxMenu->addSeparator();
            ctxMenu->addActions(l);
        }
        if (acts & ActionsTrackProp) {
            // TODO
            ctxMenu->addSeparator();
            QAction *track_properties = ctxMenu->addAction(tr("Track Properties"));
            track_properties->setObjectName("track_properties");
            ActionItem::setDefaultIcon(track_properties);
            track_properties->setEnabled(false);
        }
    }
    ctxMenu->popup(parent->mapToGlobal(QPoint(0,0)) + p);

    // Change playlist name
    // Delete playlist
    // Add new playlist
    // Duplicate playlist
    // [] Turn autosorting
    // ---
    // Cut
    // Copy
    // Paste
    // ---
    // Track properties (invalid translation?)
    // Replaygain (menu as above)
    // Convert
    // Playback (menu)
    // -/ Add to queue
    // -/ Delete from queue
    // Reload metadata
    return;
}

void ActionManager::onChangePlaylistName() {
    api->renamePlaylist(playlist_number);
}

void ActionManager::onDeletePlaylist() {
    api->removePlaylist(playlist_number);
}

void ActionManager::onAddNewPlaylist() {
    api->newPlaylist(tr("New Playlist"));
}
