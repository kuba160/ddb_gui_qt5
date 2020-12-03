#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <QCache>
#include <DBApi.h>
#include <PlaylistView.h>


class PlayItemWrapper{
public:
    PlayItemWrapper(DBApi *Api, DB_playItem_t *item);
    ~PlayItemWrapper();
    DBApi *api;
    ddb_playItem_t *it;
    int ref = 0;
};

class QueueManager : public PlaylistView {
    Q_OBJECT
public:
    QueueManager(QWidget *parent = nullptr, DBApi *Api = nullptr);
    ~QueueManager();
    static QWidget *constructor(QWidget *parent, DBApi *Api);

public slots:
    void onQueueChanged();

protected:
    void dropEvent(QDropEvent *event);
private:

    QCache<DB_playItem_t *, PlayItemWrapper> table;
    ddb_playlist_t *qplt = nullptr;
};

#endif // QUEUEMANAGER_H
