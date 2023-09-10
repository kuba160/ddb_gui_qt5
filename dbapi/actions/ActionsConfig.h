#ifndef ACTIONSCONFIG_H
#define ACTIONSCONFIG_H

#include "../DBApi.h"
#include "ActionOwner.h"
#include "ActionSpec.h"
#include <QObject>

class DBActionConfigBool : public DBAction {
    Q_OBJECT
    bool value;
    bool value_def;
    DBApi *api;
    QString config_str;
    bool config_default;
    bool current_value;
public:
    DBActionConfigBool(QObject *parent, DBApi *api, ActionSpec spec);
    // apply
    virtual QHash<QString, QVariant> contextualize(PlayItemIterator &context) const override;
    virtual bool apply(PlayItemIterator &context) override;

public slots:
    void onConfigChanged();
};

class DBActionShuffle : public DBAction {
    Q_OBJECT
    DBApi *api;
    ddb_shuffle_t shuffle;
public:
    DBActionShuffle(QObject *parent, DBApi *Api,  ddb_shuffle_t shuffle);
    ~DBActionShuffle();
    virtual QHash<QString, QVariant> contextualize(PlayItemIterator &context) const override;
    virtual bool apply(PlayItemIterator &context) override;
};

class DBActionRepeat : public DBAction {
    Q_OBJECT
    DBApi *api;
    ddb_repeat_t repeat;
public:
    DBActionRepeat(QObject *parent, DBApi *Api,  ddb_repeat_t repeat);
    ~DBActionRepeat();
    virtual QHash<QString, QVariant> contextualize(PlayItemIterator &context) const override;
    virtual bool apply(PlayItemIterator &context) override;
};

// Action that follows given property
class DBActionPropertyBool : public DBAction {
    Q_OBJECT
    QMetaProperty prop_meta;
    QObject *prop_owner;
public:
    DBActionPropertyBool(QObject *parent, ActionSpec spec);
    //~DBActionPropertyBool();
    virtual QHash<QString, QVariant> contextualize(PlayItemIterator &context) const override;
    virtual bool apply(PlayItemIterator &context) override;

    void connectProperty(QObject *property_owner, QString prop_name);
};

class ActionsConfig : public ActionOwner
{
    Q_OBJECT
public:
    explicit ActionsConfig(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~ActionsConfig();
    virtual QList<DBAction*> getActions() override;
    virtual DBAction* getAction(QString action_id) override;

private:
    QList<DBAction*> m_actions;
};

#endif // ACTIONSCONFIG_H
