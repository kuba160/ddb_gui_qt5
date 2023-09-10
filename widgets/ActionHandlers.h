#ifndef ACTIONHANDLERS_H
#define ACTIONHANDLERS_H

#include <QMenu>
#include <QMenuBar>
#include <QObject>

#include <dbapi/DBApi.h>
#include <dbapi/Actions.h>

class ActionHandlers : QObject {
    Q_OBJECT
    DBApi *api;
    QList<QFutureWatcher<PlayItemIterator>> futures;
public:
    ActionHandlers(QWidget *parent, DBApi *Api);
};


#endif // ACTIONHANDLERS_H
