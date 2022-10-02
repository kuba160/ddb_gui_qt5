#include "PlaylistModel.h"
#include <QDebug>
#define DBAPI (manager->deadbeef)

PlaylistModel::PlaylistModel(QObject *parent, PlaylistManager *Manager, ddb_playlist_t *plt) : PlayItemModel(parent, Manager) {
    // TODO
    /*connect(api, SIGNAL(playlistContentChanged(ddb_playlist_t*)),
            this, SLOT(onPlaylistContentChanged(ddb_playlist_t*)));*/
    if (plt) {
        setPlaylist(plt);
    }
}

PlaylistModel::~PlaylistModel() {
    if (plt) {
        DBAPI->plt_unref(plt);
    }
}

ddb_playlist_t* PlaylistModel::getPlaylist() {
    return plt;
}

void PlaylistModel::setPlaylist(ddb_playlist_t *plt_new) {
    beginResetModel();
    ddb_playlist_t *plt_old = this->plt;
    this->plt = plt_new;
    if (plt_new) {
        DBAPI->plt_ref(plt_new);
    }
    if (plt_old) {
        DBAPI->plt_unref(plt_old);
    }
    emit playlistChanged(plt);
    endResetModel();
}

void PlaylistModel::onPlaylistContentChanged(ddb_playlist_t *plt_changed) {
    if (plt == plt_changed) {
        // refresh
        setPlaylist(plt);
    }
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        //qDebug() << "PlaylistModel: rowCount (err)";
        return 0;
    }
    if (plt) {
        DBAPI->pl_lock ();
        int rowCount = DBAPI->plt_get_item_count(plt, m_iter);
        DBAPI->pl_unlock ();
        return rowCount;
    }
    qDebug() << (void *)this << "PlaylistModel: rowCount (err2)";
    return 0;
}

playItemList PlaylistModel::tracks(const QList<int> &tracks) const {
    if (tracks.length() == 0 || m_playlistLock)
        return playItemList();

    playItemList list;
    foreach(int t, tracks) {
        list.append(DBAPI->plt_get_item_for_idx(plt, t, m_iter));
    }
    return list;
}

void PlaylistModel::sort(int n, Qt::SortOrder order) {
    /*
    if (m_playlistLock) {
        return;
    }
    if (plt) {
        beginResetModel();
        //DBAPI->plt_sort_v2(plt, m_iter, -1, getHeaderFormat(n).toUtf8(), order);
        endResetModel();
    }*/
}

void PlaylistModel::insertTracks(playItemList *l, int after) {
    DB_playItem_t *it;
    if (after == -1) {
        it = nullptr;
    }
    else if (after == -2) {
        it = DBAPI->plt_get_last(plt,m_iter);
    }
    else {
        if (after > DBAPI->plt_get_item_count(plt, m_iter)) {
            return;
        }
        it = DBAPI->plt_get_item_for_idx(plt,after,m_iter);
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
    // qDebug() << "moveItems:" << ind.length() << "items to be put after" << after;

    // DeaDBeeF requires uint32_t array
    uint32_t inds[ind.length()];
    int i = 0;
    foreach(int d, ind) {
        inds[i] = d;
        i++;
    }

    int lastItem = DBAPI->plt_get_item_count(plt, m_iter) - 1;
    if (after == -2) {
        after = lastItem;
    }
    DB_playItem_t *bef;
    if (after == -1) {
        bef = DBAPI->plt_get_first(plt,m_iter);
    }
    else {
        if (after > lastItem) {
            return;
        }
        bef = DBAPI->plt_get_item_for_idx(plt, after+1, m_iter);
    }

    // TODO: use beginMoveRows?
    beginResetModel();
    DBAPI->pl_lock();
    DBAPI->plt_move_items(plt, m_iter, plt, bef, inds, ind.length());
    if (bef) {
        DBAPI->pl_item_unref(bef);
    }
    DBAPI->pl_unlock();
    endResetModel();
}

void PlaylistModel::removeIndexes(QList<int> ind) {
    playItemList l = tracks(ind);
    // TODO: use beginRemoveRows?
    beginResetModel();
    foreach(DB_playItem_t *it, l) {
        DBAPI->plt_remove_item(plt,it);
    }
    endResetModel();
}
