#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QTreeWidgetItem>
#include <QClipboard>
#include <QMenuBar>
#include "DBApi.h"

class ActionItem : public QAction {
    Q_OBJECT
public:
    ActionItem(DB_plugin_action_t *);
    // text - deadbeef action text
    // object name - deadbeef action name
    // Properties:
    // Title - original text before translation
    // DBACTION - original DB_plugin_action_t pointer
    //
    // DeaDBeeF Action flags (used for all types)
    uint32_t ddb_flags;
    // DeaDBeeF Action callback (TYPE_DEADBEEF)
    DB_plugin_action_callback2_t callback2;
    DB_plugin_action_t *plug_action;

    // check if this action is nested one and needs menu creation
    bool isNested();

    static QString iconOnAction(QString action);
    static void setDefaultIcon(QAction *);
protected:
    bool nested = false;
private slots:
    void onActionTriggered();
};

class ActionManager : public QObject {
    Q_OBJECT
public:
    //Q_OBJECT
    ActionManager(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~ActionManager();



    QMenuBar *mainMenuBar = nullptr;
protected:
    enum ActionGroup {
        ActionsEmpty     = 0,
        ActionsPlayback  = 1 << 0,
        ActionsClipboard = 1 << 1,
        ActionsDelete    = 1 << 2,
        ActionsPlugins   = 1 << 3,
        ActionsCustom    = 1 << 4,
        ActionsTrackProp = 1 << 5
    };
    //
    int menuActionsAvailable(QObject *);
    static void insertActionWithName(QMenu *, QList<QAction*> *, QString name);
    QList<QAction *> defaultPlaylistActions();


    void fillMenuBar();
    void loadActions();

    QList<ActionItem *> actions;

    QPoint playItemMenuPosition;
    QPoint playlistMenuPosition;
    // playlist number playlist menu is referring to (TODO support multiple playlists?)
    int playlist_number = -1;

    DBApi *api;
public slots:
    // Create context menu in point p for playitem it
    void playItemContextMenu(QWidget *obj, QPoint p);
    // Create context menu in point p for playlist number plt
    void playlistContextMenu(QWidget *obj, QPoint p, int plt);

    // Playlist default actions
    void onChangePlaylistName();
    void onDeletePlaylist();
    void onAddNewPlaylist();
};

#endif // ACTIONMANAGER_H
