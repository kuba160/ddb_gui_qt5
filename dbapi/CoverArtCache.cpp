#include <QtConcurrent>

#include <unistd.h>
#include "CoverArtCache.h"

#define CACHE_SIZE 1000

CoverArtCache::CoverArtCache(QObject *parent, ICoverArtBackend *cover_provider) : ICoverArtCache(parent) {
    backend = cover_provider;
}

CoverArtCache::~CoverArtCache() {
    foreach(QString str, cache_refc.keys()) {
        if (cache_refc.value(str) == 0) {
            cache_refc.remove(str);
        }
        else {
            qDebug() << QString("Image (path: %1) has refc=%2!").arg(str) .arg(cache_refc.value(str));
        }
    }
    cache_path.clear();
}

CoverArtStatusFlags CoverArtCache::coverArtStatus(CoverArtRequest_t &request) {
    mut.lock();
    CoverArtStatusFlags flags = STATUS_UNAVAIL;
    if (request.type == COVER_QSTRING) {
        if (cache_path.contains(request.it)) {
            flags = STATUS_CACHED;
        }
        else if (m_processing.contains(request.it)) {
            flags = STATUS_LOADING;
        }
        else {
            flags = STATUS_MISS;
        }
    }
    mut.unlock();
    return flags;
}

QFuture<QVariant> CoverArtCache::requestCoverArt(CoverArtRequest_t &request) {
    if (request.type == COVER_QSTRING && coverArtStatus(request) & STATUS_MISS) {
        if (request.it) {
            mut.lock();
            m_processing.insert(request.it);
            CoverArtRequest_t *req = new CoverArtRequest_t(request);
            mut.unlock();
            return QtConcurrent::run(cover_art_load,this, req);
        }
        else {
            qDebug() << "Called requestCoverArt with invalid track pointer!";
        }
    }
    return QFuture<QVariant>();
}

QVariant CoverArtCache::getCoverArt(CoverArtRequest_t &request) {
    if (request.type == COVER_QSTRING) {
        if (coverArtStatus(request) & STATUS_CACHED) {
            mut.lock();
            QString str = cache_path.value(request.it);
            mut.unlock();
            if (str.isEmpty()) {
                return QVariant();
            }
            return QUrl::fromLocalFile(str);

        }
        else if (coverArtStatus(request) & STATUS_MISS) {
            requestCoverArt(request);
            return QVariant();
        }
    }
    return QVariant();
}

// Threaded static functions

QVariant CoverArtCache::cover_art_load(CoverArtCache *cac, CoverArtRequest_t *request) {
    // cache path for track
    QFuture<char *> c = cac->backend->loadCoverArt(request->it);
    c.waitForFinished();
    QString path = QString();
    if (c.result() && strlen(c.result())) {
        path = c.result();
    }
    cac->registerNewPath(*request, path);

    // clean up
    cac->m_processing.remove(request->it);
    delete request;
    return QUrl(path);
}

void CoverArtCache::registerNewPath(CoverArtRequest_t &req, QString &path) {
    mut.lock();
    QString prevValue = cache_path.value(req.it, QString());
    int refc = prevValue.isNull() ? 0 : cache_refc.value(prevValue);
    cache_refc.insert(path, refc + 1);
    cache_path.insert(req.it, path);
    emit coverArtChanged(COVER_QSTRING, req.source_id, req.userdata, req.it);
    mut.unlock();
}

void CoverArtCache::unref(CoverArtType type, QVariant value) {
    if (type == COVER_QSTRING) {
        mut.lock();
        QString str = value.toString();
        if (cache_refc.contains(str)) {
            int refc = cache_refc.value(str, -1);
            if (refc > 0) {
                cache_refc.insert(str, refc - 1);
                if (refc == 1 && cache_refc.size() > CACHE_SIZE) {
                    DB_playItem_t *it = cache_path.key(str, nullptr);
                    if (it) {
                        cache_path.remove(it);
                    }
                }
            }
            else {
                qDebug() << QString("CoverArtCache::unref: refc mismatch for path %1!").arg(str);
            }
        }
        else {
            qDebug() << QString("CoverArtCache::unref: no match found for path %1!").arg(str);
        }
        mut.unlock();
    }
}

void CoverArtCache::ref(CoverArtType type, QVariant value) {
    if (type == COVER_QSTRING) {
        mut.lock();
        QString str = value.toString();
        QHash<QString, int>::const_iterator i = cache_refc.constFind(str);
        if (i != cache_refc.cend()) {
            cache_refc.insert(str, i.value() + 1);
        }
        else {
            qDebug() << QString("CoverArtCache::ref: no match found for path %1!").arg(str);
        }
        mut.unlock();
    }
}

void CoverArtCache::track_unref(DB_playItem_t *it) {
    // remove all occurences of track
    mut.lock();
    if (cache_path.contains(it)) {
        cache_path.remove(it);
    }
    mut.unlock();
}
