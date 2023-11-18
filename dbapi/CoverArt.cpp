#include "CoverArt.h"
#include <QDebug>
#include <QVariant>
#include <QTimer>
#include <QUrl>

#include "CoverArtBackend.h"
#include "CoverArtCache.h"

#define DBAPI (this->deadbeef)

CoverArt::CoverArt(QObject *parent, DB_functions_t *api) : ICoverArtCache{parent} {
    deadbeef = api;
    cover_provider = nullptr;

    if (DBAPI->plug_get_for_id ("artwork2")) {
            qDebug() << "CoverArt: using new backend";
            cover_provider = new CoverArtNew(parent, api);
    }
    else if (DBAPI->plug_get_for_id("artwork")) {
        DB_plugin_t *p = DBAPI->plug_get_for_id("artwork");
        if (p->api_vmajor == 1) {
            qDebug() << "CoverArt: using legacy backend";
            cover_provider = new CoverArtLegacy(parent, DBAPI);
        }
        else if (p->api_vmajor == 2) {
            qDebug() << "CoverArt: using new backend";
            cover_provider = new CoverArtNew(parent,DBAPI);
        }
    }
    if (!cover_provider) {
        qDebug() << "CoverArt: backend missing!";
        return;
    }

    connect(&current_cover_path_watcher, &QFutureWatcher<char *>::finished, this, &CoverArt::current_cover_path_handler);
    DB_playItem_t *it = DBAPI->streamer_get_playing_track();
    if (cover_provider) {
        if (it) {
            current_cover_path_watcher.setFuture(cover_provider->loadCoverArt(it));
            DBAPI->pl_item_unref(it);
        }
        else {
            m_current_cover_path = QString(cover_provider->getDefaultCoverArt());
        }
        insertCoverArtCache(COVER_QSTRING, new CoverArtCache(this, cover_provider));
    }


    QTimer::singleShot(1000, this, [this]() {
        DB_playItem_t *it = DBAPI->streamer_get_playing_track();
        if (it) {
            current_cover_path_watcher.setFuture(cover_provider->loadCoverArt(it));
            DBAPI->pl_item_unref(it);
        }
    });
}

CoverArt::~CoverArt() {
    delete cover_provider;
    for (const ICoverArtCache *c : std::as_const(cache_hash)) {
        delete c;
    }
    cache_hash.clear();
}


int CoverArt::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    Q_UNUSED(p1)
    Q_UNUSED(p2)
    switch (id) {
        case DB_EV_SONGCHANGED:
            ddb_event_trackchange_t *track_change = (ddb_event_trackchange_t *)(ctx);
            if (cover_provider) {
                if (track_change->to != nullptr) {
                current_cover_path_watcher.setFuture(cover_provider->loadCoverArt(track_change->to));
                }
                else {
                    m_current_cover_path = QString(cover_provider->getDefaultCoverArt());
                    emit currentCoverPathChanged();
                }
            }
    }

    return 0;
}

void CoverArt::current_cover_path_handler() {
    const char* result = current_cover_path_watcher.result();
    if (!result) {
        result = cover_provider->getDefaultCoverArt();
    }
    m_current_cover_path = QString(result);
    emit currentCoverPathChanged();
    cover_provider->unloadCoverArt(result);
}

QUrl CoverArt::getCurrentCoverPath() {
    return QUrl::fromLocalFile(m_current_cover_path);
}

bool CoverArt::getCoverArtAvailable() {
    return cover_provider;
}

/*QFuture<char *> CoverArt::getCoverArtPath(DB_playItem_t *it) {
    return cover_provider->loadCoverArt(it);
}*/

CoverArtStatusFlags CoverArt::coverArtStatus(CoverArtRequest_t &req) {
    if (cache_hash.contains(req.type)) {
        return cache_hash.value(req.type)->coverArtStatus(req);
    }
    return STATUS_UNAVAIL;
}

QFuture<QVariant> CoverArt::requestCoverArt(CoverArtRequest_t &req) {
    if (cache_hash.contains(req.type)) {
        return cache_hash.value(req.type)->requestCoverArt(req);
    }
    return QFuture<QVariant>();
}

QVariant CoverArt::getCoverArt(CoverArtRequest_t &req) {
    if (cache_hash.contains(req.type)) {
        return cache_hash.value(req.type)->getCoverArt(req);
    }
    return QVariant();
}

bool CoverArt::insertCoverArtCache(int type, ICoverArtCache *obj) {
    if (cache_hash.contains(type)) {
        delete obj;
        return false;
    }
    cache_hash.insert(type, obj);
    obj->setParent(this);
    connect(obj, &ICoverArtCache::coverArtChanged, this, &CoverArt::coverArtChanged);
    return true;
}

qint64 CoverArt::allocateSourceId() {
    return source_id_next++;
}
