#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QObject>
#include <dbapi/DBApi.h>


class Playlist : public QObject
{
    Q_OBJECT
public:
    //explicit Playlist(QObject *parent = nullptr);
    static QObject *constructor(QWidget *parent = nullptr, DBApi *Api =nullptr);


signals:

};

#endif // PLAYLIST_H
