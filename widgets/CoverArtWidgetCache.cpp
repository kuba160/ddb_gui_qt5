#include <QtConcurrent>

#include "CoverArtWidgetCache.h"
#include <QVariant>

#define DBAPI (this->api)

#define CACHE_SIZE 100

bool operator==(const ScaledCoverEntry_t &lhs, const ScaledCoverEntry_t &rhs) noexcept {
    return lhs.path == rhs.path && lhs.size == rhs.size;
}

uint qHash(const ScaledCoverEntry_t &c, uint seed) noexcept {
    QtPrivate::QHashCombine hash;
    seed = hash(seed, c.path);
    seed = hash(seed, c.size.height());
    seed = hash(seed, c.size.width());
    return seed;
}

CoverArtWidgetCache::CoverArtWidgetCache(QObject *parent, DBApi *Api) : ICoverArtCache(parent) {
    api = Api;
}

CoverArtWidgetCache::~CoverArtWidgetCache() {
    foreach(QString str, cache_refc.keys()) {
        if (cache_refc.value(str) == 0) {
            cache_refc.remove(str);
            delete cache_img.value(str);
        }
        else {
            qDebug() << QString("Image (path: %1) has refc=%2!").arg(str) .arg(cache_refc.value(str));
        }
    }
    cache_img.clear();
}

CoverArtStatusFlags CoverArtWidgetCache::coverArtPathAvailable(CoverArtRequest_t *request) {
    CoverArtRequest_t path_request = *request;
    path_request.type = COVER_QSTRING;
    return DBAPI->playlist.coverArtStatus(path_request);
}

QVariant CoverArtWidgetCache::getCoverArtPath(CoverArtRequest_t *request) {
    CoverArtRequest_t path_request = *request;
    path_request.type = COVER_QSTRING;
    return DBAPI->playlist.getCoverArt(path_request);
}
QFuture<QVariant> CoverArtWidgetCache::requestCoverArtPath(CoverArtRequest_t *request) {
    CoverArtRequest_t path_request = *request;
    path_request.type = COVER_QSTRING;
    return DBAPI->playlist.requestCoverArt(path_request);
}

QImage* CoverArtWidgetCache::cover_art_cache_scaled(QString &path, QSize size) {
    if (cache_img.contains(path)) {
        QImage *img = cache_img.value(path);
        QImage *img_scaled = new QImage(img->scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
        return img_scaled;
    }
    return nullptr;
}

CoverArtStatusFlags CoverArtWidgetCache::coverArtStatus(CoverArtRequest_t &request) {
    CoverArtStatusFlags flags = STATUS_UNAVAIL;
    QVariant path = getCoverArtPath(&request);
    CoverArtStatusFlags path_flags = coverArtPathAvailable(&request);
    if (path_flags == STATUS_UNAVAIL) {
        return STATUS_UNAVAIL;
    }
    mut.lock();
    if (path.isValid() && request.type == COVER_QIMAGE) {
        flags = STATUS_MISS;
        if (request.size.isValid()) {
            if (cache_img_scaled.contains({path.toString(), request.size})) {
                flags = STATUS_CACHED;
            }
        }
        else {
            if (cache_img.contains(path.toString())) {
                flags =  STATUS_CACHED;
            }
        }
        //if (m_processing.contains(request.it)) {
        //    flags = STATUS_LOADING;
        //}
    }
    mut.unlock();
    return flags;
}

QFuture<QVariant> CoverArtWidgetCache::requestCoverArt(CoverArtRequest_t &request) {
    // request path first
    CoverArtStatusFlags stat = coverArtPathAvailable(&request);
    if (stat == STATUS_CACHED) {
        QString path = getCoverArtPath(&request).toString();
        if (request.size.isValid()) {
            // scaled cover
            if (!cache_img_scaled.contains({path, request.size})) {
                // enqueue scaled cover loading
                // todo
            }
        }
        else {
            // non-scaled cover
            if (!cache_img.contains(path)) {
                // enqueue cover loading
                // todo
            }
        }
    }
    else if (stat == STATUS_MISS) {
        CoverArtRequest_t *req_new = new CoverArtRequest_t(request);
        return QtConcurrent::run(cover_art_load,api,this, req_new);
    }
    return QFuture<QVariant>();
}

QVariant CoverArtWidgetCache::getCoverArt(CoverArtRequest_t &request) {
    if (request.type == COVER_QIMAGE) {
        CoverArtRequest_t path_request = request;
        path_request.type = COVER_QSTRING;
        CoverArtStatusFlags stat = DBAPI->playlist.coverArtStatus(path_request);
        if (coverArtStatus(request) & STATUS_CACHED) {
            QString path = DBAPI->playlist.getCoverArt(path_request).toString();
            if (request.size.isValid()) {
                if (cache_img_scaled.contains({path, request.size})) {
                    return QVariant::fromValue(cache_img_scaled.value({path, request.size}));
                }
                else {
                    requestCoverArt(request);
                }
            }
            else {
                if (cache_img.contains(path)) {
                    return QVariant::fromValue(cache_img.value(path));
                }
                else {
                    requestCoverArt(request);
                }
            }
        }
        else if (coverArtStatus(request) & STATUS_MISS) {
            requestCoverArt(request);
            return QVariant();
        }
    }
    return QVariant();
}

// Threaded static functions

QVariant CoverArtWidgetCache::cover_art_load(DBApi *api, CoverArtWidgetCache *cac, CoverArtRequest_t *request) {

    // cache path for track
    CoverArtStatusFlags path_status = cac->coverArtPathAvailable(request);
    QString path;
    QFuture<QVariant> path_future;
    switch(path_status) {
        case STATUS_CACHED:
            path = cac->getCoverArtPath(request).toString();
            break;
        case STATUS_MISS:
        case STATUS_PARTIAL:
            path_future = cac->requestCoverArtPath(request);
            break;
        case STATUS_UNAVAIL:
            delete request;
            return QVariant();
        case STATUS_LOADING:
            // TODO
            ;
    }

    if (!path_future.isCanceled()) {
        path_future.waitForFinished();
        path = path_future.result().toString();
    }


    if (!path.isEmpty()) {
        // quick load if original size cached
        if (request->size.isValid()) {
            QImage *img_scaled = cac->cover_art_cache_scaled(path, request->size);
            if (img_scaled) {
                cac->registerNewCover(request, path, img_scaled);
                delete request;
                return QVariant::fromValue(img_scaled);
            }
        }

        // create qimage
        QImage *img = new QImage(path);
        if (request->size.isValid()) {
            QImage *img_scaled = new QImage(img->scaled(request->size, Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
            delete img;
            cac->registerNewCover(request, path, img_scaled);
            delete request;
            return QVariant::fromValue(img_scaled);
        }
        cac->registerNewCover(request, path, img);
        delete request;
        return QVariant::fromValue(img);
    }
    return QVariant();
}

void CoverArtWidgetCache::registerNewCover(CoverArtRequest_t *req, QString &path, QImage *image) {
    mut.lock();
    if (req->size.isValid()) {
        cache_img.insert(path, image);
    }
    else {
        cache_img_scaled.insert({path, req->size}, image);
    }
    mut.unlock();
    emit coverArtChanged(COVER_QSTRING, req->source_id, req->userdata, req->it);
}

void CoverArtWidgetCache::unref(CoverArtType type, QVariant value) {
    /*
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
                qDebug() << QString("CoverArtWidgetCache::unref: refc mismatch for path %1!").arg(str);
            }
        }
        else {
            qDebug() << QString("CoverArtWidgetCache::unref: no match found for path %1!").arg(str);
        }
        mut.unlock();
    }*/
}

void CoverArtWidgetCache::ref(CoverArtType type, QVariant value) {
    /*
    if (type == COVER_QSTRING) {
        mut.lock();
        QString str = value.toString();
        QHash<QString, int>::const_iterator i = cache_refc.constFind(str);
        if (i != cache_refc.cend()) {
            cache_refc.insert(str, i.value() + 1);
        }
        else {
            qDebug() << QString("CoverArtWidgetCache::ref: no match found for path %1!").arg(str);
        }
        mut.unlock();
    }*/
}

void CoverArtWidgetCache::track_unref(DB_playItem_t *it) {
}
