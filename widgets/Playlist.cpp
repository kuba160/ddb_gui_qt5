#include "Playlist.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>

#include <QtGui.h>
#include <QtGuiSettings.h>

#include "MainWindow.h"

Playlist::Playlist(QWidget *parent, DBApi *Api) : PlaylistView(parent,Api) {
    // Set current playlist
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    playlistModel.setPlaylist(plt);
    DBAPI->plt_unref(plt);

    // Follow current playlist
    connect(api,SIGNAL(playlistChanged()),this,SLOT(onPlaylistChanged()));
}

QWidget * Playlist::constructor(QWidget *parent, DBApi *Api) {
    return new Playlist(parent, Api);
}
