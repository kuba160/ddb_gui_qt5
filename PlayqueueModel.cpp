#include "PlayqueueModel.h"

#include "QtGui.h"
#include "MainWindow.h"
#include "DeadbeefTranslator.h"
#include "QtGui.h"

#undef DBAPI
#define DBAPI api->deadbeef

PlayqueueModel::PlayqueueModel(QObject *parent, DBApi *Api) : PlayItemTableModel(parent, Api) {
    connect(api, SIGNAL(queueChanged()),
            this, SLOT(onQueueChanged()));

    // plt - temporary playlist for sorting tracks
    plt =  DBAPI->plt_alloc("playqueuemodel");
}

PlayqueueModel::~PlayqueueModel() {
    DBAPI->plt_free(plt);
}

void PlayqueueModel::onQueueChanged() {
    // TODO: use more efficient signaling
    beginResetModel();
    //qDebug() << "PlayqueueModel: onQueueChanged";
    endResetModel();
}

int PlayqueueModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    if (parent.isValid()) {
        return 0;
    }
    return DBAPI->playqueue_get_count();
}

playItemList PlayqueueModel::tracks(const QList<int> &track_list) const {
    if (track_list.length() == 0) {
        return playItemList();
    }
    playItemList list;
    foreach(int t, track_list) {
        if (t < DBAPI->playqueue_get_count())
            list.append(DBAPI->playqueue_get_item(t));
    }
    return list;
}

void PlayqueueModel::insertTracks(playItemList *l, int after) {
    if (after == -2) {
        beginInsertRows(QModelIndex(),DBAPI->playqueue_get_count(), DBAPI->playqueue_get_count());
        foreach(DB_playItem_t *it, *l) {
            DBAPI->playqueue_push(it);
        }
        endInsertRows();
        return;
    }
    if (after == -1) {
        after = 0;
    }
    beginInsertRows(QModelIndex(),after, after + l->length());
    foreach(DB_playItem_t *it, *l) {
        DBAPI->playqueue_insert_at(after++,it);
    }
    endInsertRows();
}

void PlayqueueModel::moveIndexes(QList<int> ind, int after) {
    //qDebug() << "moveItems:" << ind.length() << "items to be put after" << after;

    // Idea behind this:
    // 1. Have a playlist with copied tracks that currently are in the queue
    // 2. Have a map that maps newly created tracks into older ones
    // 3. Queue is emptied and playlist is sorted
    // 4. Playlist is read and using map old tracks are reinserted into queue
    // TODO: if track in queue has only one ref, it might get deleted?

    beginResetModel();
    QHash<DB_playItem_t*,DB_playItem_t*> ht;
    while(DBAPI->playqueue_get_count()) {
        DB_playItem_t *it = DBAPI->playqueue_get_item(DBAPI->playqueue_get_count()-1);
        DB_playItem_t *it_copy = DBAPI->pl_item_alloc();
        DBAPI->pl_item_copy(it_copy, it);
        ht.insert(it_copy,it);
        DBAPI->plt_insert_item(plt,nullptr,it_copy);

        // Item being taken out of queue
        DBAPI->playqueue_remove_nth(DBAPI->playqueue_get_count()-1);
        DBAPI->pl_item_unref(it);
    }

    // DeaDBeeF requires uint32_t array
    uint32_t inds[ind.length()];
    int i = 0;
    foreach(int d, ind) {
        inds[i] = d;
        i++;
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
            // TODO: leak?
            return;
        }
        bef = DBAPI->plt_get_item_for_idx(plt, after+1, PL_MAIN);
    }

    // Rearrange temporary playlist
    DBAPI->pl_lock();
    DBAPI->plt_move_items(plt, PL_MAIN, plt, bef, inds, ind.length());
    if (bef) {
        DBAPI->pl_item_unref(bef);
    }
    DBAPI->pl_unlock();

    // Reinsert tracks
    for(int i = 0; i < DBAPI->plt_get_item_count(plt, PL_MAIN); i++) {
        DB_playItem_t *it = DBAPI->plt_get_item_for_idx(plt,i, PL_MAIN);
        if (ht.contains(it)) {
            DBAPI->playqueue_push(ht.value(it));
        }
        DBAPI->pl_item_unref(it);
    }
    DBAPI->plt_clear(plt);
    endResetModel();
}

void PlayqueueModel::removeIndexes(QList<int> ind) {
    // TODO: use beginRemoveRows?
    beginResetModel();
    foreach(int i, ind) {
        DBAPI->playqueue_remove_nth(i);
    }
    endResetModel();
}
