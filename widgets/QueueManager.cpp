#include "QueueManager.h"

QueueManager::QueueManager(QWidget *parent, DBApi *Api) : PlaylistView(parent,Api) {
    // Disable drop (todo?)
    //setAcceptDrops(false);

    // Create new playlist
    qplt = DBAPI->plt_alloc("qtQueueManager");
    DBAPI->plt_ref(qplt);

    playlistModel.setPlaylist(qplt);

    // Subscribe for queue add/remove
    connect(api,SIGNAL(queueChanged()),this, SLOT(onQueueChanged()));
    // Override default header? (in constructor)

    // Override queue actions
    add_to_playback_queue->disconnect();
    remove_from_playback_queue->disconnect();
    paste->disconnect();
    cut->disconnect();
    connect(add_to_playback_queue, SIGNAL(triggered()), this, SLOT(onAddToPlaybackQueue()));
    connect(remove_from_playback_queue, SIGNAL(triggered()), this, SLOT(onRemoveFromPlaybackQueue()));
    connect(paste, SIGNAL(triggered()), this, SLOT(onPaste()));
    connect(cut, SIGNAL(triggered()), this, SLOT(onCut()));
    setProperty("queue_remove_always_enabled",true);
    setProperty("pluginActionsDisabled", true);
    playlistModel.setProperty("queueManager", true);
}

QueueManager::~QueueManager() {
    //
}



QWidget * QueueManager::constructor(QWidget *parent,DBApi *Api) {
    return new QueueManager(parent,Api);
}

void QueueManager::onQueueChanged() {
    if (inDropEvent) {
        return;
    }
    // Reload data
    playlistModel.modelBeginReset();
    DBAPI->pl_lock();
    DBAPI->plt_clear(qplt);

    // Keep playItems that are in playqueue
    int i; int n = DBAPI->playqueue_get_count();
    QCache<DB_playItem_t *, PlayItemWrapper> steal_table;

    for (i = 0; i < n; i++) {
        DB_playItem_t *it = DBAPI->playqueue_get_item(i);
        if (!steal_table.contains(it) && cache_table.contains(it)) {
            // steal
            steal_table.insert(it,cache_table.take(it));
        }
        DBAPI->pl_item_unref(it);
    }
    // Clear rest of pleayItems
    cache_table.clear();

    // Rebuild playlist, use playItems from steal_table if possible (duplicates need to have multiple playItems)
    DB_playItem_t *prev = nullptr;
    for (i = 0; i < n; i++) {
        DB_playItem_t *it = DBAPI->playqueue_get_item(i);
        PlayItemWrapper *piw = nullptr;
        if (steal_table.contains(it)) {
            piw = steal_table.take(it);
        }
        else {
            piw = new PlayItemWrapper(api, it);
        }
        cache_table.insert(it,piw);
        DBAPI->plt_insert_item(qplt, prev, piw->it);
        prev = piw->it;
        DBAPI->pl_item_unref(it);
    }
    DBAPI->pl_unlock();
    playlistModel.modelEndReset();
}

void QueueManager::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasFormat("deadbeef/playitems")) {
        int row = indexAt(event->pos()).row();

        if (event->source() == this) {
            // Adjust position based on indicator
            // TODO: fix when track is sometimes off by one or something, idk, too tired
            if(row != -1 && dropIndicatorPosition() == QAbstractItemView::BelowItem) {
                //qDebug() << "item below, down";
                //++row == -1 ? row = 0 : 0;
                ++row;

                //() << "row" << row << "=>" << ++row;
            }
            else if (dropIndicatorPosition() == QAbstractItemView::AboveItem) {
                //qDebug() << "row" << row << "=>" << --row;
                //row == -1 ? row = 0 : 0;
                --row == -1 ? row = 0 : 0;
            }
            QList<int> list_int;
            QByteArray ba = event->mimeData()->data("playlistmodel/rows");
            qDebug() << "lenght" << ba.length();
            QDataStream ds(ba);
            ds >> list_int;
            /* (int i = 0; i < list_int.length(); i++) {
                if (i == row)
                    return;
            }*/

            QList<DB_playItem_t *> sel;
            QList<DB_playItem_t *> rest;
            int i;
            // Remove selected
            for (i = 0; i < list_int.length(); i++) {
                sel.append(DBAPI->playqueue_get_item(list_int[i]));
            }
            for (i = 0; i < sel.length(); i++) {
                DBAPI->playqueue_remove(sel[i]);
            }
            for (i = 0; i < DBAPI->playqueue_get_count(); i++) {
                rest.append(DBAPI->playqueue_get_item(i));
            }

            DBAPI->playqueue_clear();

            qDebug() << "Have " << sel.length() << "sel and " << rest.length() << "rest";
            qDebug() << "row is" << row;
            // Insert selected at row
            i = 0;
            while (sel.length() || rest.length()) {
                qDebug() << "loop i=" << i;
                if ((!sel.length()) || (rest.length() && (row == -1 || row != i))) {
                    if (rest.length()) {
                        qDebug() << "inserting rest" << rest[0];
                        DBAPI->playqueue_push(rest[0]);
                        DBAPI->pl_item_unref(rest[0]);
                        rest.removeFirst();
                    }
                }
                else {
                    if (sel.length()) {
                        qDebug() << "inserting sel" << sel[0];
                        while (sel.length()) {
                            DBAPI->playqueue_push(sel[0]);
                            DBAPI->pl_item_unref(sel[0]);
                            sel.removeFirst();
                        }
                    }
                }
                i++;
            }
            //DBAPI->playqueue_clear();
            inDropEvent = false;
            onQueueChanged();
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else {
            QList<DB_playItem_t *> list = api->mime_playItems(event->mimeData());
            inDropEvent = false;
            for (int i = 0; i <list.length(); i++) {
                // insert at cursor
                if (row != -1) {
                    DBAPI->playqueue_insert_at(row++,list[i]);
                }
                else {
                    DBAPI->playqueue_push(list[i]);
                }
            }
            event->setDropAction(Qt::CopyAction);
            event->accept();

        }
    }
    event->ignore();
}

void QueueManager::onAddToPlaybackQueue() {
    QModelIndexList indexes = this->selectedIndexes();
    QSet<int> rows;
    for (int i = 0; i < indexes.length(); i++) {
        int row = indexes.at(i).row();
        if (!rows.contains(row)) {
            DB_playItem_t *it = DBAPI->playqueue_get_item(row);
            if (it) {
                DBAPI->playqueue_push(it);
                DBAPI->pl_item_unref(it);
            }
            rows.insert(row);
        }
    }
}
void QueueManager::onRemoveFromPlaybackQueue() {
    QModelIndexList indexes = this->selectedIndexes();
    QSet<int> rows;
    int subt = 0;
    for (int i = 0; i < indexes.length(); i++) {
        int row = indexes.at(i).row();
        if (!rows.contains(row)) {
            // asssume row == position in queue
            DBAPI->playqueue_remove_nth(row - subt);
            subt++;
            rows.insert(row);
        }
    }
}

void QueueManager::onCut() {
    onCopy();
    QModelIndexList qmil = selectionModel()->selectedRows();
    QSet<int> l_i;
    QList<DB_playItem_t *> l;
    for (int i = 0; i < qmil.length(); i++) {
        if (l_i.contains(qmil.at(i).row())) {
            DB_playItem_t *it = DBAPI->plt_get_item_for_idx(playlistModel.getPlaylist(), qmil[i].row(), PL_MAIN);
            l.append(it);
        }
    }
    for (int i = 0; l.length(); i++) {
        DBAPI->playqueue_remove(l.at(i));
    }
}

void QueueManager::onPaste() {
    if (api->clipboard->mimeData()->hasFormat("deadbeef/playitems")) {
        QList<DB_playItem_t *> l = api->mime_playItems(api->clipboard->mimeData());
        int row =  indexAt(menu_pos).row();
        for (int i = 0; i < l.length(); i++) {
            //menu_pos

            if (row == -1) {
                DBAPI->playqueue_push(l.at(i));
            }
            else {
                DBAPI->playqueue_insert_at(row++,l.at(i));
            }
        }
    }
}

PlayItemWrapper::PlayItemWrapper(DBApi *Api, DB_playItem_t *item) {
    api = Api;
    it = DBAPI->pl_item_alloc();
    Api->deadbeef->pl_item_copy(it,item);
    //DBAPI->pl_item_ref(it);`
}

PlayItemWrapper::~PlayItemWrapper() {
    if (ref != 0) {
        qDebug() << QString("PlayItemWrapper: ref for item %1 equals %2!") .arg(reinterpret_cast<quintptr>(it)) .arg(ref);
    }
    DBAPI->pl_item_unref(it);
    //delete it;
}
