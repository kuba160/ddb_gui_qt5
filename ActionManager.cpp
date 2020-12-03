#include "ActionManager.h"
#include "DBApi.h"
#include <QMenu>


ActionManager::ActionManager (QObject *parent, DBApi *Api) : QObject(parent) {
    api = Api;

    loadActions();
    qDebug() << actions.length();
}

ActionManager::~ActionManager() {
    int i;
    for (i = 0; i < actions.length(); i++) {
        delete actions[i];
    }
    for (i = 0; i < actions_main.length(); i++) {
        delete actions_main[i];
    }
    if (playItemMenu) {
        delete playItemMenu;
    }


}

void ActionManager::loadActions() {
    QList <ActionItem *> *lp = &actions;

    DB_plugin_t **pluglist = DBAPI->plug_get_list();
    int i = 0;
    while (pluglist[i]) {
        if (pluglist[i]->get_actions) {

            if (pluglist[i]->id && strcmp(pluglist[i]->id,"hotkeys") == 0) {
                lp = &actions_main;
            }
            else {
                lp = &actions;
            }
            // append all actions
            DB_plugin_action_t *itr = pluglist[i]->get_actions(nullptr); // kinda off implementation :(
            while(itr) {
                ActionItem *n = new ActionItem;
                n->title = itr->title;
                n->name = itr->name;
                n->callback2 = itr->callback2;
                n->ddb_action = itr;
                n->ddb_flags = itr->flags;
                n->is_dir = false;
                lp->append(n);
                itr = itr->next;
            }
        }
        i++;
    }
}


void ActionManager::playItemContextMenu(QPoint p, DB_playItem_t *it) {
    if (!playItemMenu) {
        playItemMenu = new QMenu();

        // generate
        int i;
        for (i = 0; i < actions_main.length(); i++) {
            ActionItem *ac = actions_main[i];
            if ( (ac->ddb_flags & DB_ACTION_MULTIPLE_TRACKS) &&
                !(ac->ddb_flags & DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST) &&
                !(ac->ddb_flags & DB_ACTION_COMMON)) {
                const char *title;
                if ((title = strchr(ac->title.toUtf8(),'/')) == nullptr) {
                    title = ac->title.toUtf8();
                }
                else {
                    title++;
                }

                if (!ac->action) {
                    ac->action = new QAction(_(title),playItemMenu);
                    connect(ac->action,SIGNAL(triggered(bool)),this, SLOT(onAction(bool)));
                    playItemMenu->addAction(ac->action);
                }
            }
        }

        // cut/copy/paste
        // todo
        playItemMenu->addSeparator();
        playItemMenu->addAction(_("Cut"));
        playItemMenu->addAction(_("Copy"));
        playItemMenu->addAction(_("Paste"));
        playItemMenu->addSeparator();

        // plugin menus
        for (i = 0; i < actions.length(); i++) {
            ActionItem *ac = actions[i];
            if ( (ac->ddb_flags & DB_ACTION_MULTIPLE_TRACKS) &&
                !(ac->ddb_flags & DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST) &&
                !(ac->ddb_flags & DB_ACTION_COMMON)) {
                const char *title;
                if ((title = strchr(ac->title.toUtf8(),'/')) == nullptr) {
                    title = ac->title.toUtf8();
                }
                else {
                    title++;
                }

                if (!ac->action) {
                    ac->action = new QAction(title,playItemMenu);
                    connect(ac->action,SIGNAL(triggered(bool)),this, SLOT(onAction(bool)));
                    playItemMenu->addAction(ac->action);
                }
            }
        }

        // Properties
        playItemMenu->addSeparator();
        playItemMenu->addAction(_("Track Properties"));
    }
    DB_playItem_t *playItemMenuRef = it;
    playItemMenu->move(p);
    playItemMenu->show();

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

    //QMenu menu(nullptr);
    //menu.add
    return;
}
/*
void ActionManager::playlistContextMenu(QPoint p, int n) {
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
}*/

ActionTreeItem::ActionTreeItem(QObject *parent, DBApi *Api, DB_plugin_action_t *action) {
    if (!action) {
        is_dir = true;
        ddb_flags = 0;
        callback2 = nullptr;
        return;
    }
    // interate for action
    DB_plugin_action_t *a_itr = action;
    while (a_itr) {
        qDebug() << a_itr->title;
        // iterate for separators in title
        QRegularExpression re("\\/(?<!\\\\\\/)");
        QStringList strlist = QString(a_itr->title).split(re);
        int i;
        ActionTreeItem *p = this;
        ActionTreeItem *child_append = nullptr;
        for (i = 0; i < strlist.length() - 1; i++) {
            // iterate through current children to find same dir
            // if not, create new one
            int j;
            for (j = 0; j < p->childCount(); j++) {
                if (p->child(j)->text(0) == strlist[i]) {
                    child_append = static_cast<ActionTreeItem*>(p->child(j));
                    break;
                }
            }
            if (child_append) {
                qDebug() << "Subchild found " + child_append->text(0);
                p = child_append;
            }
            else {
                ActionTreeItem *new_child = new ActionTreeItem();
                new_child->setText(0, strlist[i].replace("\\/","/"));
                new_child->title = "";
                new_child->is_dir = true;
                p->addChild(new_child);
                p = new_child;
            }
        }
        // Append the final item
        ActionTreeItem *final_child = new ActionTreeItem();
        final_child->title = strlist[i];
        final_child->setText(0, strlist[i].replace("\\/","/"));
        final_child->is_dir = false;
        final_child->ddb_flags = action->flags;
        final_child->name = action->name;
        final_child->callback2 = action->callback2;
        p->addChild(final_child);

        a_itr = a_itr->next;
    }
}

void ActionManager::onAction(bool checked) {
    QObject *sendr = sender();

    // find action
    int i;
    for (i = 0; i < actions_main.length(); i++) {
        QAction *ac = actions_main[i]->action;
        if (ac == sendr) {
            // todo
            actions_main[i]->callback2(actions_main[i]->ddb_action,DDB_ACTION_CTX_SELECTION);
            return;
        }
    }
    for (i = 0; i < actions.length(); i++) {
        QAction *ac = actions[i]->action;
        if (ac == sendr) {
            // todo
            actions[i]->callback2(actions[i]->ddb_action,DDB_ACTION_CTX_SELECTION);
            return;
        }
    }
    qDebug() << "ActionManager: onAction failed to find action!" << Qt::endl;
}
