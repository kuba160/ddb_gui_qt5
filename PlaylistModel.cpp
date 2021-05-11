#include "PlaylistModel.h"

#include "QtGui.h"
#include "MainWindow.h"
#include "DeadbeefTranslator.h"

PlaylistModel::PlaylistModel(QObject *parent, DBApi *Api) : PlayItemModel(parent,Api) {
    connect(api, SIGNAL(playlistContentChanged(ddb_playlist_t*)),
            this, SLOT(onPlaylistContentChanged(ddb_playlist_t*)));
}

PlaylistModel::PlaylistModel(ddb_playlist_t *plt, QObject *parent, DBApi *Api) : PlaylistModel(parent,Api) {
    setPlaylist(plt);
}

PlaylistModel::~PlaylistModel() {
    if (plt) {
        DBAPI->plt_unref(plt);
    }
}

void PlaylistModel::setPlaylist(ddb_playlist_t *plt_new) {
    beginResetModel();
    if (plt) {
        DBAPI->plt_unref(plt);
    }
    plt = plt_new;
    if (plt) {
        DBAPI->plt_ref(plt);
    }
    endResetModel();
}

void PlaylistModel::onPlaylistContentChanged(ddb_playlist_t *plt_changed) {
    if (plt == plt_changed) {
        // refresh
        setPlaylist(plt);
    }
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if (plt) {
        DBAPI->pl_lock ();
        int rowCount = DBAPI->plt_get_item_count(plt, PL_MAIN);
        DBAPI->pl_unlock ();
        return rowCount;
    }
    return 0;
}

int PlaylistModel::trackCount() const {
    return rowCount(QModelIndex());
}

playItemList PlaylistModel::tracks(const QModelIndexList &tracks) const {
    if (tracks.length() == 0 || playlistLock())
        return playItemList();

    playItemList list;
    QList<int> rowused;
    foreach(QModelIndex t, tracks) {
        if (!rowused.contains(t.row())) {
            DB_playItem_t *it = DBAPI->plt_get_item_for_idx(plt, t.row(), PL_MAIN);
            if (it) {
                list.append(it);
            }
            rowused.append(t.row());
        }
    }
    return list;
}

playItemList PlaylistModel::tracks(const QList<int> &tracks) const {
    if (tracks.length() == 0 || playlistLock())
        return playItemList();

    playItemList list;
    foreach(int t, tracks) {
        list.append(DBAPI->plt_get_item_for_idx(plt, t, PL_MAIN));
    }
    return list;
}

DB_playItem_t * PlaylistModel::track(const QModelIndex &track) const {
    return DBAPI->plt_get_item_for_idx(plt, track.row(), PL_MAIN);
}

void PlaylistModel::sort(int n, Qt::SortOrder order) {
    if (!plt || n == -1 || n >= columns.length() ||
            columns[n]->type == HT_playing || columns[n]->format.isEmpty() || playlistLock()) {
        return;
    }
    beginResetModel();
    DBAPI->plt_sort_v2(plt, PL_MAIN, -1, columns[n]->format.toUtf8(), order);
    endResetModel();
}

void PlaylistModel::insertTracks(playItemList *l, int after) {
    DB_playItem_t *it;
    if (after == -1) {
        it = nullptr;
    }
    else if (after == -2) {
        it = DBAPI->plt_get_last(plt,PL_MAIN);
    }
    else {
        if (after > DBAPI->plt_get_item_count(plt, PL_MAIN)) {
            return;
        }
        it = DBAPI->plt_get_item_for_idx(plt,after,PL_MAIN);
    }

    DB_playItem_t *iter = it;
    beginInsertRows(QModelIndex(),after, l->length());
    foreach(DB_playItem_t *i, *l) {
        DB_playItem_t *inserted = DBAPI->plt_insert_item(plt,iter,i);
        iter = inserted;
    }
    if (it) {
        DBAPI->pl_item_unref(it);
    }
    endInsertRows();
}

void PlaylistModel::moveIndexes(QList<int> ind, int after) {
    qDebug() << "moveItems:" << ind.length() << "items to be put after" << after;
    // TODO FIX THIS
    uint32_t inds[ind.length()];
    //uint32_t inds[indices.length()];
    int i;
    for (i = 0; i < ind.length(); i++) {
        inds[i] = ind[i];
    }
    int lastItem = DBAPI->plt_get_item_count(plt, PL_MAIN) - 1;
    if (after == -2) {
        after = lastItem;
    }
    DB_playItem_t *bef;
    if (after == -1) {
        bef = DBAPI->plt_get_first(plt,PL_MAIN);
    }
    else {
        if (after > lastItem) {
            return;
        }
        bef = DBAPI->plt_get_item_for_idx(plt, after+1, PL_MAIN);
    }
    beginResetModel();
    DBAPI->pl_lock();
    DBAPI->plt_move_items(plt, PL_MAIN, plt, bef, inds, ind.length());
    if (bef) {
        DBAPI->pl_item_unref(bef);
    }
    DBAPI->pl_unlock();
    endResetModel();
    emit rowsChanged();
}

void PlaylistModel::removeIndexes(QList<int> ind) {
    playItemList l = tracks(ind);
    beginResetModel();
    foreach(DB_playItem_t *it, l) {
        DBAPI->plt_remove_item(plt,it);
    }
    endResetModel();
}


