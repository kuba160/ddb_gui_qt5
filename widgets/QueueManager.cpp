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
    // clear current playlist
    DBAPI->plt_clear(qplt);

    // fill playlist

    int n = DBAPI->playqueue_get_count();
    int i;
    DB_playItem_t *prev = nullptr;
    for (i = 0; i < n; i++) {
        DB_playItem_t *it = DBAPI->playqueue_get_item(i);
        PlayItemWrapper *piw = nullptr;
        if (table.contains(it)) {
            piw = table.object(it);
        }
        else {
            piw = new PlayItemWrapper(api,it);
            table.insert(it,piw);
        }
        DBAPI->plt_insert_item(qplt, prev, table.object(it)->it);
        prev = piw->it;
    }
    playlistModel.setPlaylist(qplt);
}

void QueueManager::dropEvent(QDropEvent *event) {
    qDebug() << "We have drop :)";
    if (event->mimeData()->hasUrls()) {
        qDebug() << "TODODODODO dropEvent in QueueManager";
        /*ddb_playlist_t *plt = playlistModel.getPlaylist();
        int count = DBAPI->plt_get_item_count(plt, PL_MAIN);
        DBAPI->plt_unref(plt);
        int row = indexAt(event->pos()).row();
        int before = (row >= 0) ? row - 1 : count - 1;
        foreach (QUrl url, event->mimeData()->urls()) {
            playlistModel.insertByURLAtPosition(url, before);
            before++;
        }*/
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if (event->mimeData()->hasFormat("medialib/tracks")) {
        QByteArray encodedData = event->mimeData()->data("medialib/tracks");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        playItemList a;
        stream >> a;
        //qDebug() <<"dropEven:" << a.list.at(0)->startsample << a.list.at(0)->endsample << a.list.at(0)->shufflerating << Qt::endl;
        qint64 i;
        ddb_playlist_t *plt = playlistModel.getPlaylist();
        for (i = a.count-1; i >= 0; i--) {
            // insert straight into queue :)
            DB_playItem_t *item_new = a.list.at(i);
            DBAPI->playqueue_insert_at(DBAPI->playqueue_get_count(),item_new);

            // TODO insert pos
            //playlistModel.insertByPlayItemAtPosition(a.list.at(i),indexAt(event->pos()).row());
            //DBAPI->plt_insert_item(plt,nullptr,a.list.at(i));
        }
        DBAPI->plt_unref(plt);
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->ignore();
    }
}


PlayItemWrapper::PlayItemWrapper(DBApi *Api, DB_playItem_t *item) {
    api = Api;
    it = DBAPI->pl_item_alloc();
    Api->deadbeef->pl_item_copy(it,item);
    ref++;
}

PlayItemWrapper::~PlayItemWrapper() {
    if (ref != 0) {
        qDebug() << QString("PlayItemWrapper: ref for item %1 equals %2!") .arg(reinterpret_cast<quintptr>(it)) .arg(ref);
    }
    delete it;
}

