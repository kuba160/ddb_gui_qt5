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
    //sel->setModel(this->selectionModel());
    connect(api,SIGNAL(playlistChanged()),this,SLOT(onPlaylistChanged()));
    connect(this->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(onSelectionChanged(const QItemSelection &, const QItemSelection &)));

    // enter / doubleclick
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(enterRelease(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));

}

Playlist::~Playlist() {
}

QWidget * Playlist::constructor(QWidget *parent, DBApi *Api) {
    return new Playlist(parent, Api);
}

void Playlist::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    ddb_playlist_t *plt = playlistModel.getPlaylist();
    QList <QModelIndex> a = selected.indexes();
    QList <int> select_int;
    int i;
    for (i = 0; i < a.length(); i++) {
        if(!select_int.contains(a[i].row())) {
            select_int.append(a[i].row());
        }
    }
    for (i = 0; i< select_int.length(); i++) {
        DB_playItem_t *pl = DBAPI->plt_get_item_for_idx(plt,select_int[i], PL_MAIN);
        if (pl) {
            DBAPI->plt_item_set_selected(plt, pl, true);
            DBAPI->action_set_playlist(plt);
            DBAPI->pl_item_unref(pl);
        }
    }

    a = deselected.indexes();
    select_int.clear();
    for (i = 0; i < a.length(); i++) {
        if(!select_int.contains(a[i].row())) {
            select_int.append(a[i].row());
        }
    }
    for (i = 0; i< select_int.length(); i++) {
        DB_playItem_t *pl = DBAPI->plt_get_item_for_idx(plt,select_int[i], PL_MAIN);
        if (pl) {
            DBAPI->pl_set_selected(pl,false);
            DBAPI->pl_item_unref(pl);
        }
    }
    DBAPI->plt_unref(plt);
}

void Playlist::onPlaylistChanged() {
    DBAPI->pl_lock();
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    playlistModel.setPlaylist(plt);
    DBAPI->plt_unref(plt);
    DBAPI->pl_unlock();
}

void Playlist::mouseDoubleClickEvent(QMouseEvent* event ) {
    if (event->button() == Qt::LeftButton) {
        QTreeView::mouseDoubleClickEvent(event);
    }

}
