#include "PlayqueueModel.h"

#include "QtGui.h"
#include "MainWindow.h"
#include "DeadbeefTranslator.h"

PlayqueueModel::PlayqueueModel(QObject *parent, DBApi *Api) : PlayItemModel(parent,Api) {
    connect(api, SIGNAL(queueChanged()),
            this, SLOT(onQueueChanged()));
    plt =  DBAPI->plt_alloc("playqueuemodel");
}

PlayqueueModel::~PlayqueueModel() {
    DBAPI->plt_free(plt);
}

void PlayqueueModel::onQueueChanged() {
    beginResetModel();
    endResetModel();
}


int PlayqueueModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return DBAPI->playqueue_get_count();
}

playItemList PlayqueueModel::tracks(const QModelIndexList &tracks) const {
    if (tracks.length() == 0 || playlistLock())
        return playItemList();

    playItemList list;
    QList<int> rowused;
    foreach(QModelIndex t, tracks) {
        if (!rowused.contains(t.row())) {
            list.append(DBAPI->playqueue_get_item(t.row()));
            rowused.append(t.row());
        }
    }
    return list;
}

playItemList PlayqueueModel::tracks(const QList<int> &tracks) const {
    if (tracks.length() == 0 || playlistLock())
        return playItemList();

    playItemList list;
    foreach(int t, tracks) {
        list.append(DBAPI->playqueue_get_item(t));
    }
    return list;
}

DB_playItem_t * PlayqueueModel::track(const QModelIndex &track) const {
    if (track.row() == -1 || track.row() >= rowCount(track)) {
        return nullptr;
    }
    return DBAPI->playqueue_get_item(track.row());
}

void PlayqueueModel::sort(int n, Qt::SortOrder order) {
    if (!DBAPI->playqueue_get_count() || n == -1 || n >= columns.length() ||
            columns[n]->type == HT_playing || columns[n]->format.isEmpty() || playlistLock()) {
        return;
    }
    beginResetModel();
    QHash<DB_playItem_t*,DB_playItem_t*> ht;
    while(DBAPI->playqueue_get_count()) {
        DB_playItem_t *it = DBAPI->playqueue_get_item(DBAPI->playqueue_get_count()-1);
        DB_playItem_t *it_copy = DBAPI->pl_item_alloc();
        DBAPI->pl_item_copy(it_copy, it);
        ht.insert(it_copy,it);
        DBAPI->plt_insert_item(plt,nullptr,it_copy);
        DBAPI->playqueue_remove_nth(DBAPI->playqueue_get_count()-1);
        DBAPI->pl_item_unref(it);
    }
    DBAPI->plt_sort_v2(plt, PL_MAIN, -1, columns[n]->format.toUtf8(), order);
    for(int i = 0; i < DBAPI->plt_get_item_count(plt, PL_MAIN); i++) {
        DB_playItem_t *it = DBAPI->plt_get_item_for_idx(plt,i, PL_MAIN);
        if (ht.contains(it))
            DBAPI->playqueue_push(ht.value(it));
        DBAPI->pl_item_unref(it);
        //DBAPI->plt_remove_item(plt,it);
    }
    DBAPI->plt_clear(plt);
    //DBAPI->plt_unref(plt);
    endResetModel();
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
    qDebug() << "moveItems:" << ind.length() << "items to be put after" << after;

    beginResetModel();
    QHash<DB_playItem_t*,DB_playItem_t*> ht;
    while(DBAPI->playqueue_get_count()) {
        DB_playItem_t *it = DBAPI->playqueue_get_item(DBAPI->playqueue_get_count()-1);
        DB_playItem_t *it_copy = DBAPI->pl_item_alloc();
        DBAPI->pl_item_copy(it_copy, it);
        ht.insert(it_copy,it);
        DBAPI->plt_insert_item(plt,nullptr,it_copy);
        DBAPI->playqueue_remove_nth(DBAPI->playqueue_get_count()-1);
        DBAPI->pl_item_unref(it);
    }

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

    DBAPI->pl_lock();
    DBAPI->plt_move_items(plt, PL_MAIN, plt, bef, inds, ind.length());
    if (bef) {
        DBAPI->pl_item_unref(bef);
    }
    DBAPI->pl_unlock();

    for(int i = 0; i < DBAPI->plt_get_item_count(plt, PL_MAIN); i++) {
        DB_playItem_t *it = DBAPI->plt_get_item_for_idx(plt,i, PL_MAIN);
        if (ht.contains(it))
            DBAPI->playqueue_push(ht.value(it));
        DBAPI->pl_item_unref(it);
        //DBAPI->plt_remove_item(plt,it);
    }
    DBAPI->plt_clear(plt);
    endResetModel();
    emit rowsChanged();
}

void PlayqueueModel::removeIndexes(QList<int> ind) {
    beginResetModel();
    foreach(int i, ind) {
        DBAPI->playqueue_remove_nth(i);
    }
    endResetModel();
}


