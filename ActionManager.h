#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QTreeWidgetItem>
#include <QClipboard>
#include "DBApi.h"

class ActionItem {
public:

    QString title;
    QString name;
    uint32_t ddb_flags;
    DB_plugin_action_callback2_t callback2;
    DB_plugin_action_t *ddb_action;
    bool is_dir;
    QString plugin;
    QAction *action;
};

class ActionManager : public QObject {
    Q_OBJECT
public:
    //Q_OBJECT
    ActionManager(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~ActionManager();

    void loadActions();

protected:
    QList<ActionItem *> actions_main;
    QList<ActionItem *> actions;

    QMenu *playItemMenu = nullptr;
    QPoint playItemMenuPosition;
    QList<QAction *> clipboard_actions;
    DB_playItem_t *playItemMenuRef = nullptr;

    DBApi *api;
    QClipboard *clipboard;
public slots:
    // Create context menu in point p for playitem it
    void playItemContextMenu(QWidget *obj, QPoint p);
    //void playItemContextMenu(QPoint p, QList<DB_playItem_t *> it_list);
    // Create context menu in point p for playlist number n
    //void playlistContextMenu(QPoint p, int n);

private slots:
    void cut(bool);
    void copy(bool);
    void paste(bool);

private slots:
    void onAction(bool);
};



class ActionTreeItem: public QTreeWidgetItem, public DBWidget {
    //Q_OBJECT
public:
    ActionTreeItem(QObject *parent = nullptr, DBApi *Api = nullptr, DB_plugin_action_t *action = nullptr);
    QString title;
    QString name;
    uint32_t ddb_flags;
    DB_plugin_action_callback2_t callback2;
    bool is_dir;
};

#endif // ACTIONMANAGER_H
