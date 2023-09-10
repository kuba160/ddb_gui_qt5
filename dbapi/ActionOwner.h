#ifndef ACTIONOWNER_H
#define ACTIONOWNER_H

#include "Actions.h"
#include <QObject>

class DBAction;

class ActionOwner : public QObject
{
    Q_OBJECT
public:
    explicit ActionOwner(QObject *parent = nullptr);

    virtual QList<DBAction*> getActions() = 0;
    virtual DBAction* getAction(QString action_id) = 0;
public slots:
    virtual void actionSafeToDeleted(QString action_id) {}
signals:
    void actionCreated(QString action_id);
    void actionAboutToBeDeleted(QString action_id);

};

#endif // ACTIONOWNER_H
