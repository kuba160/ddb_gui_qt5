#include "Playlist.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>

#include <QtGui.h>
#include <QtGuiSettings.h>

#include "MainWindow.h"

Playlist::Playlist(QWidget *parent, DBApi *Api) : PlaylistView(parent,Api,new PlaylistModel(Api,Api)) {
    // Set current playlist
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    qobject_cast<PlaylistModel *>(model())->setPlaylist(plt);
    DBAPI->plt_unref(plt);

    // Follow current playlist
    connect(api,SIGNAL(playlistChanged()),this,SLOT(onPlaylistChanged()));
    // enter / doubleclick
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(enterRelease(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));
    // jump to current playlist
    connect(api, SIGNAL(jumpToCurrentTrack()), this, SLOT(jumpToCurrentTrack()));

}

void Playlist::trackDoubleClicked(QModelIndex index) {
    if (index.isValid()) {
        api->playTrackByIndex(index.row());
    }
}

Playlist::~Playlist() {
}

QWidget * Playlist::constructor(QWidget *parent, DBApi *Api) {
    return new Playlist(parent, Api);
}

void Playlist::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
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
    qobject_cast<PlaylistModel *>(pi_model)->setPlaylist(plt);
    // scroll?
    // autoscroll for current track
    jumpToCurrentTrack();
    DBAPI->plt_unref(plt);
    DBAPI->pl_unlock();
}

void Playlist::mouseDoubleClickEvent(QMouseEvent* event ) {
    if (event->button() == Qt::LeftButton) {
        QTreeView::mouseDoubleClickEvent(event);
    }

}

void Playlist::jumpToCurrentTrack() {
    // todo fix this
    //ddb_playlist_t *plt = DBAPI->plt_get_curr();
    //if (DBAPI->streamer_get_current_playlist() == plt) {
        DB_playItem_t *it = DBAPI->streamer_get_playing_track();
        if (it) {
            scrollTo(model()->index(DBAPI->pl_get_idx_of(it),0),QAbstractItemView::PositionAtTop);
            DBAPI->pl_item_unref(it);
        }
    //}
}
