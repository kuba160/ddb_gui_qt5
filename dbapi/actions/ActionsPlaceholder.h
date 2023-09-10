#ifndef ACTIONSPLACEHOLDER_H
#define ACTIONSPLACEHOLDER_H

#include "ActionOwner.h"
#include "ActionSpec.h"
#include <QObject>


class DBActionPlaceholder : public DBAction {
    Q_OBJECT
public:
    DBActionPlaceholder(QObject *parent, struct ActionSpec spec);
    virtual QHash<QString, QVariant> contextualize(PlayItemIterator &context) const override;
    virtual bool apply(PlayItemIterator &context) override;
};

class ActionsPlaceholder : public ActionOwner
{
    Q_OBJECT
public:
    explicit ActionsPlaceholder(QObject *parent = nullptr);

    virtual QList<DBAction*> getActions() override;
    virtual DBAction* getAction(QString action_id) override;
private:
    QList<DBAction*> m_actions;
};

#endif // ACTIONSPLACEHOLDER_H
