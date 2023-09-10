#ifndef ACTIONS_H
#define ACTIONS_H

#include <QObject>
#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <deadbeef/deadbeef.h>

#include <QHash>

#include "PlayItemIterator.h"
#include "ActionOwner.h"
#include <QList>

class ActionsModel;
class ActionsDefault;
class ActionOwner;

class DBAction : public QObject {
    Q_OBJECT
public:
    DBAction(QObject *parent) : QObject(parent){};

    // standard ddb parameters
    QString title; // action name (NOT PATH);
    QStringList path;
    QString action_id; // id
    
    enum ActionAccepts {
        ACTION_ARG_NONE = 1 << 0,
        ACTION_ARG_TRACK = 1 << 1,
        ACTION_ARG_TRACKS = 1 << 2,
        ACTION_ARG_PLAYLIST = 1 << 3,
        ACTION_ARG_ALL = (ACTION_ARG_NONE | ACTION_ARG_TRACK |
                          ACTION_ARG_TRACKS | ACTION_ARG_PLAYLIST)
    };
    Q_ENUM(ActionAccepts);
    enum ActionLocations {
        ACTION_LOC_HOTKEY = 1 << 0,
        ACTION_LOC_TRACK_CONTEXT = 1 << 1, // item(s) context
        ACTION_LOC_PLAYLIST_CONTEXT = 1 << 2,
        ACTION_LOC_MENUBAR = 1 << 3,
        ACTION_LOC_ALL = (ACTION_LOC_HOTKEY | ACTION_LOC_TRACK_CONTEXT |
                          ACTION_LOC_PLAYLIST_CONTEXT | ACTION_LOC_MENUBAR)

    };
    Q_ENUM(ActionLocations);

    uint16_t accepts;
    uint16_t locations;

    QHash<QString, QVariant> properties_const;

    // contextualize
    virtual QHash<QString, QVariant> contextualize(PlayItemIterator &context) const {return properties_const;};
    // apply
    virtual bool apply(PlayItemIterator &context) = 0;

signals:
    void actionApplied(PlayItemIterator &pit);
    virtual void actionPropertiesChanged(void);
};

class Actions : public QObject
{
    Q_OBJECT
public:
    explicit Actions(QObject *parent);

    void registerActionOwner(ActionOwner*);
    // unregisterActionOwner

private:

    QList<ActionOwner *> m_action_owners;
    QHash<uint32_t, ActionsModel*> m_prototypes;
    uint32_t prototype_counter = 0;

public slots:

    DBAction* getAction(QString action_id);
    QStringList getActions();
    QString getActionTitle(QString action_id);
    QHash<QString, QVariant> getActionContext(QString id, PlayItemIterator &context);
    bool execAction(QString action, PlayItemIterator context = PlayItemIterator(false));


    uint32_t registerPrototype(uint32_t location_filter, uint32_t accepts_filter, QString config);

    QJsonArray parsePrototype(uint32_t prototype, PlayItemIterator pit);
    QJsonArray parsePrototype(uint32_t prototype);
    QAbstractItemModel * prototypeModel(uint32_t prototype);



signals:
    void prototypeInvalidated(uint32_t prototype);
    void actionsAdded();
    void actionAdded(QString action_id);
    void actionDeleted(QString action_id);

protected:


    const int filter_flags[4] = { DBAction::ACTION_LOC_HOTKEY, DBAction::ACTION_LOC_TRACK_CONTEXT,
                                      DBAction::ACTION_LOC_PLAYLIST_CONTEXT, DBAction::ACTION_LOC_MENUBAR };

    void rebuildMenu(DBAction::ActionLocations);


    QString getDefaultConfig(DBAction::ActionLocations);




    static constexpr int locToNum(DBAction::ActionLocations loc) {
        switch(loc) {
            case DBAction::ACTION_LOC_HOTKEY:
                return 0;
            case DBAction::ACTION_LOC_TRACK_CONTEXT:
                return 1;
            case DBAction::ACTION_LOC_PLAYLIST_CONTEXT:
                return 2;
            case DBAction::ACTION_LOC_MENUBAR:
                return 3;
            default:
                return 0;
        }
    }
signals:

};

// actions comment

// Actions provide unified system for maintaining deadbeef and player related actions
// They can be distributed in different locations such as application menu, right click track menu etc.
// Each action in addition to standard action description (similar to deadbeef) can be contextualized.
// This means that an action can provide extra parameters (or override default) when the context is different.
// Example: "Add to Playqueue" can be replaced with "Add 2 songs to Playqueue" if the context is 2 songs.
// Example 2: "Remove from Playqueue" will be disabled if selected tracks are not in playqueue.
//
// Actions maintains models for each menu such as hotkey, track menu, playlist menu or menubar
// Each menu is generated from the config which decides the order, separators or visibility of each action.
// Models can export its internal tree structure into json format that can be parsed for generating the menu
// independent of the platform.

#endif // ACTIONS_H
