#include "ActionManager.h"
#include "DBApi.h"
#include <QMenu>


ActionManager::ActionManager (QObject *parent, DBApi *Api) : QObject(parent) {
    api = Api;

    DB_plugin_t **list = DBAPI->plug_get_list();
    int i = 0;
    while (list[i]) {
        if (list[i]->get_actions) {
            actions.append(ActionItem::getActions(list[i]->get_actions(nullptr)));
        }
        i++;
    }
    qDebug() << actions.length();
}

ActionManager::~ActionManager() {
    int i;
    for (i = 0; i < actions.length(); i++) {
        delete actions[i];
    }
}

QList <ActionItem *> ActionItem::getActions(DB_plugin_action_t *action) {
    QList <ActionItem *> list;

    DB_plugin_action_t *itr = action;
    while(itr) {
        ActionItem *n = new ActionItem;
        n->title = action->title;
        n->name = action->name;
        n->callback2 = action->callback2;
        n->ddb_flags = action->flags;
        n->is_dir = false;
        list.append(n);
        itr = action->next;
    }
    return list;
}

/*
void ActionManager::playItemContextMenu(QPoint p, DB_playItem_t *it) {
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
