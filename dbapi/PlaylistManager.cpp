#include "PlaylistManager.h"

#include "models/PlaylistBrowserModel.h"
#include "models/PlaylistModel.h"
#include "models/PlayqueueModel.h"
#include "models/PlayItemTableProxyModel.h"

#include <QDebug>

#define DBAPI (this->deadbeef)

PlaylistManager::PlaylistManager(QObject *parent, DB_functions_t *api)
    : CoverArt(parent, api)
{
    deadbeef = api;
    QVariant c;
    c.setValue(this);
    setProperty("_dbapi_cover", QVariant::fromValue(this));

    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    m_current = new PlaylistModel(this, this, plt);
    DBAPI->plt_unref(plt);

    m_queue = new PlayqueueModel(this, this);
    m_list = new PlaylistBrowserModel(this, this);
    m_current_item = new CurrentPlayItemModel(this, this);

    qDebug() << m_current->data(m_current->index(0,0));
}

PlaylistManager::~PlaylistManager() {
    delete m_current;
    delete m_queue;
    delete m_list;
    delete m_current_item;
}

QAbstractItemModel* PlaylistManager::getCurrentPlaylist() {
    return m_current;
}

int PlaylistManager::getCurrentPlaylistIdx() {
    return DBAPI->plt_get_curr_idx();
}

void PlaylistManager::setCurrentPlaylistIdx(int plt) {
    if (plt != DBAPI->plt_get_curr_idx()) {
        DBAPI->plt_set_curr_idx(plt);

        ddb_playlist_t *ddb_plt = DBAPI->plt_get_curr();
        m_current->setPlaylist(ddb_plt);
        DBAPI->plt_unref(ddb_plt);
        emit currentPlaylistChanged();

        // TODO just make PlaylistModel take int...
        //ddb_playlist_t *plt = DBAPI->plt_get_curr();
        //static_cast<PlaylistModel *>(m_current)->setPlaylist(plt);
        //DBAPI->plt_unref(plt);
    }
}

QAbstractItemModel* PlaylistManager::getQueue() {
    return m_queue;
}

QAbstractItemModel* PlaylistManager::getList() {
    return m_list;
}

QAbstractItemModel* PlaylistManager::getCurrentItem() {
    return m_current_item;
}


int PlaylistManager::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    switch(id) {
        case DB_EV_SONGCHANGED:
        case DB_EV_PAUSED:
        case DB_EV_STOP:
            emit statusRowChanged();
            break;
        case DB_EV_TRACKINFOCHANGED:
             // detect queue
             if (p1 == DDB_PLAYLIST_CHANGE_PLAYQUEUE) {
                 emit statusRowChanged();
             }
            break;
        case DB_EV_PLAYLISTSWITCHED:
            ddb_playlist_t *ddb_plt = DBAPI->plt_get_curr();
            m_current->setPlaylist(ddb_plt);
            DBAPI->plt_unref(ddb_plt);
            emit currentPlaylistChanged();
            break;

    }
    CoverArt::pluginMessage(id, ctx, p1, p2);
    return 0;
}

PlayItemTableProxyModel* PlaylistManager::createTableProxy(QObject *parent) {
    return new PlayItemTableProxyModel(parent);
}

void PlaylistManager::queueAppend(QVariant mime) {
    if (mime.value<QMimeData *>()) {
        m_queue->dropMimeData(mime.value<QMimeData *>(), Qt::CopyAction, -2, 0, QModelIndex());
        emit statusRowChanged();
        delete mime.value<QMimeData*>();
    }
    else {
        qDebug() <<" mime data error";
    }
}
