#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "DBApi.h"
#include "PlaylistView.h"

class Playlist : public PlaylistView {
    Q_OBJECT

public:
    Playlist(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QWidget *constructor(QWidget *parent, DBApi *Api);
};

#endif // PLAYLIST_H
