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
}

QueueManager::~QueueManager() {
    //
}



QWidget * QueueManager::constructor(QWidget *parent,DBApi *Api) {
    return new QueueManager(parent,Api);
}

void QueueManager::onQueueChanged() {
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
        QList<DB_playItem_t *> list = api->mime_playItems(event->mimeData());
        int row = indexAt(event->pos()).row();
        if (event->source() == this) {

            QList<int> rows;
            int i;
            for (i = 0; i < list.length(); i++) {
                rows.append(DBAPI->plt_get_item_idx(qplt,list[i],PL_MAIN));
                DBAPI->pl_item_ref(list[i]);
            }
            playlistModel.moveItems(rows,row);
            // Remove selected
            int shift = 0;
            for (i = 0; i < rows.length(); i++) {
                qDebug() << QString("Removing at %1") .arg(rows[i] + shift);
                //DBAPI->playqueue_remove_nth(rows[i] + shift++);
            }
            // Insert selected at row
            for (i = 0; i < list.length(); i++) {
                qDebug() << QString("Inserting at %1") .arg(row);
                if (row == -1) {
                    DBAPI->playqueue_push(list[i]);
                }
                else {
                    DBAPI->playqueue_insert_at(row++,list[i]);
                }
            }
            //DBAPI->playqueue_clear();
            onQueueChanged();
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
        else {
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

