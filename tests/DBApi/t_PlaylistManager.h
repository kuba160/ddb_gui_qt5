#ifndef T_PLAYLISTMANAGER_H
#define T_PLAYLISTMANAGER_H

#include <QObject>
#include "dbapi/DBApi.h"

class t_PlaylistManager : public QObject
{
    Q_OBJECT
public:
    explicit t_PlaylistManager(DBApi *Api);

private:
    DBApi *api;

private slots:

signals:
};

#endif // T_PLAYLISTMANAGER_H
