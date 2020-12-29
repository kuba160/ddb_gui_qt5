#include "ActionManager.h"
#include "DBApi.h"
#include <QMenu>
#include <QGuiApplication>
#include "MainWindow.h"
#include "DefaultActions.h"

ActionManager::ActionManager (QObject *parent, DBApi *Api) : QObject(parent) {
    api = Api;
    clipboard = QGuiApplication::clipboard();
    DefaultActions *da = new DefaultActions(Api);

    mainMenuBar = da->getDefaultMenuBar();
    QList<QMenu *> l = da->findChildren<QMenu *>();
    // translate
    // TODO fix this
    //qDebug() << l[0]->menuAction()->text() << " ->" << _(l[0]->menuAction()->text().toUtf8().constData());
    // l[0]->menuAction()->setText(_(l[0]->menuAction()->text().toUtf8().constData()));
    QList<QAction *> acs = da->findChildren<QAction *>();
    for (int j = 0; j < acs.length(); j++) {
        if (!acs[j]->text().isEmpty()) {
            //() << acs[j]->text().toUtf8() << "R->" <<_(acs[j]->text().toUtf8());
            //acs[j]->setText(_(acs[j]->text().toUtf8().constData()));
        }
    }
    loadActions();
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
        // child of window
        //delete playItemMenu;
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
                n->action_type = ActionItem::TYPE_VIRTUAL;
                lp->append(n);
                itr = itr->next;
            }
        }
        i++;
    }
}


void ActionManager::playItemContextMenu(QWidget *parent, QPoint p) {
    if (!playItemMenu) {
        playItemMenu = new QMenu(parent);

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
                    // special icons
                    if (ac->name == "add_to_playback_queue") {
                        ac->action->setIcon(QIcon::fromTheme("list-add"));
                    }
                    else if (ac->name == "remove_from_playback_queue") {
                        ac->action->setIcon(QIcon::fromTheme("list-remove"));
                    }
                    else if (ac->name == "reload_metadata") {
                        ac->action->setIcon(QIcon::fromTheme("view-refresh"));
                    }

                }
                // do not allow to remove from playback queue if track is not queued
                // TODO DBAPI->pl_is_selected
            }
        }

        // cut/copy/paste
        // todo
        playItemMenu->addSeparator();
        QAction *add;
        connect(add = playItemMenu->addAction(QIcon::fromTheme("edit-cut"), _("Cut")),SIGNAL(triggered(bool)),this,SLOT(cut(bool)));
        clipboard_actions.append(add);
        connect(add = playItemMenu->addAction(QIcon::fromTheme("edit-copy"),_("Copy")),SIGNAL(triggered(bool)),this,SLOT(copy(bool)));
        clipboard_actions.append(add);
        connect(add = playItemMenu->addAction(QIcon::fromTheme("edit-paste"),_("Paste")),SIGNAL(triggered(bool)),this,SLOT(paste(bool)));
        clipboard_actions.append(add);
        playItemMenu->addSeparator();

        // TODO INSERT "Delete", ReplayGain and "Refresh Cover"

        // plugin menus
        for (i = 0; i < actions.length(); i++) {
            ActionItem *ac = actions[i];
            if (!(ac->ddb_flags & (DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST | DB_ACTION_COMMON))) {
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
    // update clipboard actions
    DBWidget *widget = dynamic_cast<DBWidget *>(playItemMenu->parent());
    if (widget->canCopy()) {
        clipboard_actions[0]->setEnabled(true);
        clipboard_actions[1]->setEnabled(true);
    }
    else {
        clipboard_actions[0]->setEnabled(false);
        clipboard_actions[1]->setEnabled(false);
    }
    const QMimeData *data = QGuiApplication::clipboard()->mimeData();
    if (data && widget->canPaste(data)) {
        clipboard_actions[2]->setEnabled(true);
    }
    else {
        clipboard_actions[2]->setEnabled(false);
    }


    playItemMenu->move(parent->mapToGlobal(QPoint(0,0)) + p);
    playItemMenuPosition = parent->mapToGlobal(QPoint(0,0)) + p;
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
    return;
}

void ActionManager::playlistContextMenu(QWidget *parent, QPoint p, int n) {
    // TODO
    if (!playlistMenu) {
        playlistMenu = new QMenu(parent);

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
                    ac->action = new QAction(_(title),playlistMenu);
                    connect(ac->action,SIGNAL(triggered(bool)),this, SLOT(onAction(bool)));
                    playlistMenu->addAction(ac->action);
                    // special icons
                    if (ac->name == "add_to_playback_queue") {
                        ac->action->setIcon(QIcon::fromTheme("list-add"));
                    }
                    else if (ac->name == "remove_from_playback_queue") {
                        ac->action->setIcon(QIcon::fromTheme("list-remove"));
                    }
                    else if (ac->name == "reload_metadata") {
                        ac->action->setIcon(QIcon::fromTheme("view-refresh"));
                    }

                }
                // do not allow to remove from playback queue if track is not queued
                // TODO DBAPI->pl_is_selected
            }
        }

        // cut/copy/paste
        // todo
        playlistMenu->addSeparator();
        QAction *add;
        connect(add = playlistMenu->addAction(QIcon::fromTheme("edit-cut"), _("Cut")),SIGNAL(triggered(bool)),this,SLOT(cut(bool)));
        clipboard_actions.append(add);
        connect(add = playlistMenu->addAction(QIcon::fromTheme("edit-copy"),_("Copy")),SIGNAL(triggered(bool)),this,SLOT(copy(bool)));
        clipboard_actions.append(add);
        connect(add = playlistMenu->addAction(QIcon::fromTheme("edit-paste"),_("Paste")),SIGNAL(triggered(bool)),this,SLOT(paste(bool)));
        clipboard_actions.append(add);
        playlistMenu->addSeparator();

        // TODO INSERT "Delete", ReplayGain and "Refresh Cover"

        // plugin menus
        for (i = 0; i < actions.length(); i++) {
            ActionItem *ac = actions[i];
            if (!(ac->ddb_flags & (DB_ACTION_EXCLUDE_FROM_CTX_PLAYLIST | DB_ACTION_COMMON))) {
                const char *title;
                if ((title = strchr(ac->title.toUtf8(),'/')) == nullptr) {
                    title = ac->title.toUtf8();
                }
                else {
                    title++;
                }

                if (!ac->action) {
                    ac->action = new QAction(title,playlistMenu);
                    connect(ac->action,SIGNAL(triggered(bool)),this, SLOT(onAction(bool)));
                    playlistMenu->addAction(ac->action);
                }
            }
        }

        // Properties
        playlistMenu->addSeparator();
        playlistMenu->addAction(_("Track Properties"));
    }
    // update clipboard actions
    DBWidget *widget = dynamic_cast<DBWidget *>(playlistMenu->parent());
    if (widget->canCopy()) {
        clipboard_actions[0]->setEnabled(true);
        clipboard_actions[1]->setEnabled(true);
    }
    else {
        clipboard_actions[0]->setEnabled(false);
        clipboard_actions[1]->setEnabled(false);
    }
    const QMimeData *data = QGuiApplication::clipboard()->mimeData();
    if (data && widget->canPaste(data)) {
        clipboard_actions[2]->setEnabled(true);
    }
    else {
        clipboard_actions[2]->setEnabled(false);
    }


    playItemMenu->move(parent->mapToGlobal(QPoint(0,0)) + p);
    playItemMenuPosition = parent->mapToGlobal(QPoint(0,0)) + p;
    playItemMenu->show();
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

ActionTreeItem::ActionTreeItem(ActionTreeItem *parent, DBApi *Api, DB_plugin_action_t *action) : QTreeWidgetItem(parent),
                                                                                                 DBWidget(nullptr,Api) {
    if (!action) {
        action_type = ActionItem::TYPE_VIRTUAL;
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
                new_child->action_type = TYPE_VIRTUAL;
                p->addChild(new_child);
                p = new_child;
            }
        }
        // Append the final item
        ActionTreeItem *final_child = new ActionTreeItem();
        final_child->title = strlist[i];
        final_child->setText(0, strlist[i].replace("\\/","/"));
        final_child->action_type = ActionItem::TYPE_QTGUI; // ?? DEADBEEF TYPE ??
        final_child->ddb_flags = action->flags;
        final_child->name = action->name;
        final_child->callback2 = action->callback2;
        p->addChild(final_child);

        a_itr = a_itr->next;
    }
}

void ActionManager::onAction(bool checked) {
    Q_UNUSED(checked)
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
    qDebug() << "ActionManager: onAction failed to find action!" << ENDL;
}


void ActionManager::cut(bool triggered) {
    Q_UNUSED(triggered);
    DBWidget *widget = dynamic_cast<DBWidget *>(playItemMenu->parent());
    QMimeData *data = widget->cut();
    if(data) {
        clipboard->setMimeData(data);
    }
}

void ActionManager::copy(bool triggered) {
    Q_UNUSED(triggered);
    // todo - have one general menu
    DBWidget *widget = dynamic_cast<DBWidget *>(playItemMenu->parent());
    QMimeData *data = widget->copy();
    if(data) {
        QGuiApplication::clipboard()->setMimeData(data);
    }
}


void ActionManager::paste(bool triggered) {
    Q_UNUSED(triggered);
    DBWidget *widget = dynamic_cast<DBWidget *>(playItemMenu->parent());
    const QMimeData *data = QGuiApplication::clipboard()->mimeData();
    if (data && widget->canPaste(data)) {
        widget->paste(data, playItemMenuPosition);
        clipboard->clear();
    }
}
