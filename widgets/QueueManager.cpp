#include "QueueManager.h"

QueueManager::QueueManager(QWidget *parent, DBApi *Api, PlayItemTableModel *ptm_arr) : PlaylistView(parent,Api,ptm_arr) {
    // properties ... not the most elegant way
    pi_model->setProperty("queue_remove_always_enabled",true);
    setProperty("pluginActionsDisabled", true);
    pi_model->setProperty("queueManager", true);

    setModel(ptm_arr);
}

void QueueManager::dropEvent(QDropEvent *event) {
    // small changes compared to PlaylistView
    if (event->mimeData()->hasFormat("deadbeef/playitems")) {
        int row = indexAt(event->pos()).row();
        // Adjust position based on indicator
        if (row == -1) {
            row = -2;
        }
        if(row != -1 && dropIndicatorPosition() == QAbstractItemView::AboveItem) {
            row--;
        }
        if (event->source() == this) {
            // Move items inside
            QModelIndexList sel = selectedIndexes();
            QList<int> rows;
            foreach(QModelIndex i, sel) {
                if (!rows.contains(i.row())) {
                    rows.append(i.row());
                }
            }
            pi_model->moveIndexes(rows,row);
        }
        else {
            // Insert foreign items
            QList<DB_playItem_t *>list = api->mime_playItems(event->mimeData());
            pi_model->insertTracks(&list,row);
        }
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

QWidget * QueueManager::constructor(QWidget *parent,DBApi *Api) {
    PlayqueueModel *ptm = qobject_cast<PlayqueueModel *>(Api->getQueueModel());
    return new QueueManager(parent,Api, ptm);
}

