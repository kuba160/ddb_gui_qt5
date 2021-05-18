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
    // Selection update into deadbeef
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onSelectionChanged(QItemSelection,QItemSelection)));
    // Selection update from deadbeef
    connect(api, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
    // restore cursor
    int cursor = DBAPI->conf_get_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8(), -1);
    //setCurrentIndex()
    if (cursor != -1) {
        setCurrentIndex(model()->index(cursor, 0, QModelIndex()));
    }
    // restore selection
    onSelectionChanged();
}

void Playlist::trackDoubleClicked(QModelIndex index) {
    if (index.isValid()) {
        api->playTrackByIndex(index.row());
    }
}

Playlist::~Playlist() {
    // save cursor
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    if (plt) {
        int cursor = DBAPI->plt_get_cursor(plt, PL_MAIN);
        DBAPI->conf_set_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8().constData(), cursor);
        DBAPI->plt_unref(plt);
    }
}

QWidget * Playlist::constructor(QWidget *parent, DBApi *Api) {
    return new Playlist(parent, Api);
}

void Playlist::onSelectionChanged() {
    // update selection to represent deadbeef selection status
    QItemSelection sel;
    int count = pi_model->trackCount();
    for (int i = 0; i < count ; i++) {
        DB_playItem_t *it = DBAPI->pl_get_for_idx(i);
        if (it) {
            if (DBAPI->pl_is_selected(it)) {
                QModelIndex it_sel = pi_model->index(i,0,QModelIndex());
                sel.select(it_sel,it_sel);
            }
            DBAPI->pl_item_unref(it);
        }
    }
    selectionModel()->select(sel,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void Playlist::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    // update deadbeef selection status to match gui selection
    QList<QModelIndex> a = selected.indexes();
    QVector<int> select_int;
    int i;
    for (i = 0; i < a.length(); i++) {
        if(!select_int.contains(a[i].row())) {
            select_int.append(a[i].row());
        }
    }
    for (i = 0; i< select_int.length(); i++) {
        DB_playItem_t *pl = DBAPI->pl_get_for_idx(select_int[i]);
        if (pl) {
            if (i == 0) {
                DBAPI->pl_set_cursor(PL_MAIN, select_int[i]);
                DBAPI->conf_set_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8(), select_int[i]);
            }
            DBAPI->pl_set_selected(pl, true);
            //DBAPI->action_set_playlist(plt);
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
        DB_playItem_t *pl = DBAPI->pl_get_for_idx(select_int[i]);
        if (pl) {
            DBAPI->pl_set_selected(pl,false);
            DBAPI->pl_item_unref(pl);
        }
    }
}

void Playlist::onPlaylistChanged() {
    DBAPI->pl_lock();
    ddb_playlist_t *plt_new = DBAPI->plt_get_curr();
    qobject_cast<PlaylistModel *>(pi_model)->setPlaylist(plt_new);
    DBAPI->plt_unref(plt_new);

    // restore cursor
    int cursor = DBAPI->conf_get_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8(), -1);
    if (cursor != -1) {
        setCurrentIndex(model()->index(cursor, 0, QModelIndex()));
    }
    // restore selection
    QItemSelection is;
    for (int i = 0; i < model()->rowCount(); i++) {
        DB_playItem_t *it = pi_model->track(model()->index(i,0,QModelIndex()));
        if (it) {
            if (DBAPI->pl_is_selected(it)) {
                is.select(model()->index(i,0,QModelIndex()),model()->index(i,model()->columnCount(QModelIndex())-1,QModelIndex()));
            }
            DBAPI->pl_item_unref(it);
        }
    }
    selectionModel()->select(is,QItemSelectionModel::Select);
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
