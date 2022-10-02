#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <QObject>
#include <QIdentityProxyModel>
#include <dbapi/DBApi.h>


class QueueManager : public QObject
{
    Q_OBJECT
public:
    //explicit Playlist(QObject *parent = nullptr);
    static QObject *constructor(QWidget *parent = nullptr, DBApi *Api =nullptr);


signals:

};

class QueueProxyModel : public QIdentityProxyModel {
public:
    QueueProxyModel(QObject *parent, QAbstractItemModel *model);
};

#endif // QUEUEMANAGER_H
