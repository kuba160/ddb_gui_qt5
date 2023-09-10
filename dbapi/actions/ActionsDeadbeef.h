#ifndef ACTIONSDEADBEEF_H
#define ACTIONSDEADBEEF_H

#include "../DBApi.h"
#include <ActionOwner.h>

class DBActionImported : public DBAction {
    Q_OBJECT
    DB_functions_t* deadbeef;
public:
    // TODO SHOULD TAKE PARENT ARGUMENT
    DBActionImported(DB_functions_t* deadbeef, DB_plugin_action_t *);

    virtual QHash<QString, QVariant> contextualize(PlayItemIterator &context) const override;
    virtual bool apply(PlayItemIterator &context) override;


    static QString iconOnAction(const QString action);
    static uint16_t actionAcceptsFromFlags(uint32_t flags);
    static uint16_t actionLocationsFromFlags(uint32_t flags);

private:
    DB_plugin_action_t *action_original;
};

class ActionsDeadbeef : public ActionOwner
{
public:
    explicit ActionsDeadbeef(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~ActionsDeadbeef();
    virtual QList<DBAction*> getActions() override;
    virtual DBAction* getAction(QString action_id) override;

private:
    QList<DBAction*> m_actions;
};

#endif // ACTIONSDEADBEEF_H
