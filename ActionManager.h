#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QTreeWidgetItem>
#include "DBApi.h"

class ActionItem {
public:
    static QList <ActionItem *> getActions(DB_plugin_action_t *);
    QString title;
    QString name;
    uint32_t ddb_flags;
    DB_plugin_action_callback2_t callback2;
    bool is_dir;
};

class ActionManager : public QObject {
    Q_OBJECT
public:
    //Q_OBJECT
    ActionManager(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~ActionManager();

protected:
    QList<ActionItem *> actions;

    DBApi *api;
public slots:
    // Create context menu in point p for playitem it
    //void playItemContextMenu(QPoint p, DB_playItem_t *it);
    // Create context menu in point p for playlist number n
    //void playlistContextMenu(QPoint p, int n);
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
