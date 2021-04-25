#ifndef QUEUEMANAGER_H
#define QUEUEMANAGER_H

#include <DBApi.h>
#include <PlaylistView.h>
#include <PlayqueueModel.h>

class QueueManager : public PlaylistView {
    Q_OBJECT
public:
    QueueManager(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QWidget *constructor(QWidget *parent, DBApi *Api);
protected:
    void dropEvent(QDropEvent *event);
};

#endif // QUEUEMANAGER_H
