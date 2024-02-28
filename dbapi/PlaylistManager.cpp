#include "PlaylistManager.h"

#include "models/PlaylistBrowserModel.h"
#include "models/PlaylistModel.h"
#include "models/PlayqueueModel.h"
#include "models/PlayItemTableProxyModel.h"
#include "models/PlayItemFilterModel.h"


#include "ItemImporter.h"
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
    m_current_search = new PlayItemFilterModel(this);
    m_current_search->setSourceModel(m_current);
    DBAPI->plt_unref(plt);

    m_queue = new PlayqueueModel(this, this);
    m_list = new PlaylistBrowserModel(this, this);
    m_current_item = new CurrentPlayItemModel(this, this);

    qDebug() << m_current->data(m_current->index(0,0));


    DB_plugin_t *medialib = DBAPI->plug_get_for_id("medialib");
    if (medialib) {
        DB_mediasource_t *ml = static_cast<DB_mediasource_t*>((void*)medialib);
        m_medialib = new MediasourceModel(this, ml, "medialib");
        m_medialib_config = new MedialibConfig(this,ml, m_medialib->getSource());
        connect(m_medialib_config, &MedialibConfig::foldersChanged, this, &PlaylistManager::medialibFoldersChanged);
    }
}

PlaylistManager::~PlaylistManager() {
    delete m_current;
    delete m_queue;
    delete m_list;
    delete m_current_item;
    delete m_medialib;
    delete m_medialib_config;
}

QAbstractItemModel* PlaylistManager::getCurrentPlaylist() {
    return m_current;
}

QAbstractItemModel* PlaylistManager::getCurrentPlaylistSearch() {
    return m_current_search;
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

QAbstractItemModel* PlaylistManager::getMedialib() {
    return m_medialib;
}

QStringList PlaylistManager::getMedialibFolders() {
    return m_medialib_config->getFolders();
}

void PlaylistManager::setMedialibFolders(QStringList l) {
    m_medialib_config->setFolders(l);
}



int PlaylistManager::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    switch(id) {
        case DB_EV_SONGCHANGED:
        case DB_EV_PAUSED:
        case DB_EV_STOP:
            emit statusRowChanged();
            break;
        case DB_EV_PLAYLISTCHANGED:
            //ddb_playlist_change_t i = (ddb_playlist_change_t) p1;
            switch((ddb_playlist_change_t) p1) {
                case DDB_PLAYLIST_CHANGE_CREATED:
                case DDB_PLAYLIST_CHANGE_DELETED:
                case DDB_PLAYLIST_CHANGE_TITLE:
                    m_list->refreshPlaylist();
                case DDB_PLAYLIST_CHANGE_PLAYQUEUE:
                    emit statusRowChanged();
                default:
                    break;
            }
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

MediasourceModel* PlaylistManager::createMediasource(QObject *parent, QString plug_id, QString conf_name) {
    DB_plugin_t *plug = DBAPI->plug_get_for_id(plug_id.toUtf8().constData());
    if (!plug) {
        qDebug() << "Failed to create mediasource for plugin" << plug_id;
        return nullptr;
    }
    if (plug->type != DB_PLUGIN_MEDIASOURCE) {
        qDebug() << "Failed to create mediasource for plugin" << plug_id << ", plugin not a mediasource type!";
        return nullptr;
    }
    return new MediasourceModel(parent, static_cast<DB_mediasource_t*>((void *) plug), conf_name);
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

QFuture<PlayItemIterator> PlaylistManager::runFileImport(QStringList files) {
    return ItemImporter::runFileImport(files);
}

QFuture<PlayItemIterator> PlaylistManager::runFolderImport(QStringList folders) {
    return ItemImporter::runFolderImport(folders);
}

QFuture<PlayItemIterator> PlaylistManager::runPlaylistImport(QStringList playlists) {
    return ItemImporter::runPlaylistImport(playlists);
}


