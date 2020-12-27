#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QTreeWidgetItem>
#include <QClipboard>
#include <QMenuBar>
#include "DBApi.h"

class ActionItem {
public:
    enum ActionItemType {

        // Not an action
        TYPE_VIRTUAL = 1,
        // DeaDBeeF hotkeys action
        TYPE_DEADBEEF,
        // QtGui Action (QAction)
        TYPE_QTGUI
    };
    // Type
    ActionItemType action_type;

    // Action title/path (deadbef path like "File\\/Open")
    QString title;
    // Internal action name
    QString name;
    // DeaDBeeF Action flags (used for all types)
    uint32_t ddb_flags;
    // DeaDBeeF Action callback (TYPE_DEADBEEF)
    DB_plugin_action_callback2_t callback2;
    // DeaDBeeF original action pointer (TYPE_DEADBEEF)
    DB_plugin_action_t *ddb_action;
    // DeaDBeeF Action plugin owner (TYPE_DEADBEEF)
    char *plugin_name;
    // QtGui Action plugin owner (TYPE_QTGUI)
    QString plugin_name_qstring;
    // QtGui Action pointer (TYPE_QTGUI, this will be generated automatically for TYPE_DEADBEEF)
    QAction *action;
    // Action text (from QAction *action)
    QString *action_text;
    QString action_text_untranslated;
};

class ActionManager : public QObject {
    Q_OBJECT
public:
    //Q_OBJECT
    ActionManager(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~ActionManager();

    void loadActions();

    QMenuBar *mainMenuBar = nullptr;
protected:

    QList<ActionItem *> actions_main;
    QList<ActionItem *> actions;

    QMenu *playItemMenu = nullptr;
    QPoint playItemMenuPosition;
    QList<QAction *> clipboard_actions;
    DB_playItem_t *playItemMenuRef = nullptr;

    QMenu *playlistMenu = nullptr;
    QPoint playlistMenuPosition;

    DBApi *api;
    QClipboard *clipboard;
public slots:
    // Create context menu in point p for playitem it
    void playItemContextMenu(QWidget *obj, QPoint p);
    //void playItemContextMenu(QPoint p, QList<DB_playItem_t *> it_list);
    // Create context menu in point p for playlist number n
    void playlistContextMenu(QWidget *obj, QPoint p, int n);

private slots:
    void cut(bool);
    void copy(bool);
    void paste(bool);

private slots:
    void onAction(bool);
};



class ActionTreeItem: public QTreeWidgetItem, public DBWidget, public ActionItem {
    //Q_OBJECT
public:
    ActionTreeItem(ActionTreeItem *parent = nullptr, DBApi *Api = nullptr, DB_plugin_action_t *action = nullptr);
};

#endif // ACTIONMANAGER_H
