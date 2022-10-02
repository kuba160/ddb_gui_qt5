#ifndef ACTIONS_H
#define ACTIONS_H

#include <QObject>
#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <deadbeef/deadbeef.h>

class ActionsModel;

class DBAction : public QObject {
    Q_OBJECT
public:
    DBAction(QObject *parent) : QObject(parent){};

    enum ActionExecuteMode {
        ACTION_MAIN = DDB_ACTION_CTX_MAIN,
        ACTION_SELECTION = DDB_ACTION_CTX_SELECTION,
        // NOTE: starting with API 1.8, plugins should be using the
        // action_get_playlist function for getting the playlist pointer.
        ACTION_PLAYLIST = DDB_ACTION_CTX_PLAYLIST,
        ACTION_NOWPLAYING = DDB_ACTION_CTX_NOWPLAYING,
    };
    Q_ENUM(ActionExecuteMode);

    QString title;
    QString icon;
    QString action_id;
    QString path; // full unprocessed action path
    bool enabled; // enabled for given tracks, updated with buildtrackmenu
    qulonglong flags;
    // todo callback
    virtual void actionExecute(ActionExecuteMode) const = 0;
};

class DBActionImported : public DBAction {
    Q_OBJECT
    DB_functions_t* deadbeef;
public:
    DBActionImported(DB_functions_t* deadbeef, DB_plugin_action_t *);
    // Properties:
    // name/object name - internal action name
    // title - action title or subgroup title
    // path - full action path
    // flags - deadbeef flags;


    // DBACTION - original DB_plugin_action_t pointer
    //
    // DeaDBeeF Action flags (used for all types)
    // DeaDBeeF Action callback (TYPE_DEADBEEF)
    DB_plugin_action_callback2_t callback2;
    DB_plugin_action_t *plug_action;

    static QString iconOnAction(const QString action);

public slots:
    virtual void actionExecute(ActionExecuteMode role) const override;
};

class ActionContext : public QObject {
    Q_OBJECT
    friend class Actions;
public:
    // create playlist action context
    ActionContext(ddb_playlist_t* plt);
    // create playitem list action context
    ActionContext(QList<DB_playItem_t*> tracks);
    // create main or nowplaying action context
    ActionContext(bool nowplaying = false);
    ActionContext() : ActionContext(false) {};

    ~ActionContext();

    // executes given DBAction with current context
    void executeForAction(DBAction* action);

protected:
    static DB_functions_t* deadbeef;
    DBAction::ActionExecuteMode mode;
    ddb_playlist_t* plt;
    QList<DB_playItem_t *> it_list;
};

class ActionsBuilder {
public:
    virtual QVariant returnValue() = 0;
    virtual ActionsBuilder * createSubMenu(QString &title) = 0;
    virtual void insertAction(DBAction *action) = 0;
    virtual void insertSeparator() = 0;
};

typedef ActionsBuilder * (*actionsBuilderConstructor)(QObject *parent, ActionContext *context);

class Actions : public QObject
{
    Q_OBJECT
public:
    explicit Actions(QObject *parent, DB_functions_t *Api);


    Q_PROPERTY(QAbstractItemModel* actions READ getActionsModel CONSTANT);

    QAbstractItemModel* getActionsMenuModel() const;
    QAbstractItemModel *getActionsModel() const;

    bool execAction(QString &action, ActionContext &context);


    bool registerActionsBuilder(QString name, actionsBuilderConstructor builder);

    QVariant buildActionMenu(QObject *parent, QString name, ActionContext *context);
    QVariant buildTrackMenu(QObject *parent, QString name, ActionContext *context);

protected:
    DB_functions_t *deadbeef;
    ActionsModel *m_actions;
    ActionsModel *m_actions_menu;
    ActionsModel *m_actions_track;
    ActionsModel *m_actions_playlist;
    QHash<QString, DBAction *> m_actions_hash;
    QHash<QString, actionsBuilderConstructor> m_actions_builders;

    void buildActionMenuIter(ActionsBuilder *, QModelIndex &);
    void buildTrackMenuIter(ActionsBuilder *, QModelIndex &);
    //DBAction createAction(QAbstractItemModel *, int row, QModelIndex &parent);


    bool trackMenuExcludeAction(DBAction *action);
signals:

};

/*
class ActionsJsonBuilder : public ActionsBuilder, public QObject {
public:
    ActionsJsonBuilder(QObject *parent);
    virtual ActionsBuilder * createSubMenu(QString &title) override;
    virtual void insertAction(DBAction *action) override;
    QJsonObject json_obj;
    QJsonArray submenus;
    QJsonArray actions;
    QByteArray toJson(QJsonDocument::JsonFormat format = QJsonDocument::JsonFormat::Indented);
};*/


#endif // ACTIONS_H
