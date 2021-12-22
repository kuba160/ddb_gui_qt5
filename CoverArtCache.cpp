#include <QtConcurrent>

#include <unistd.h>
#include "CoverArtCache.h"
#include "DBApi.h"
#include "QtGui.h"

#define CACHE_SIZE 100

extern DB_functions_t *deadbeef_internal;

bool operator==(const coverSearch &lhs, const coverSearch &rhs) noexcept {
    return lhs.path == rhs.path && lhs.size == rhs.size;
}

uint qHash(const coverSearch &c, uint seed) noexcept {
    QtPrivate::QHashCombine hash;
    seed = hash(seed, c.path);
    seed = hash(seed, c.size.height());
    seed = hash(seed, c.size.width());
    return seed;
}

CoverArtCache::CoverArtCache(QObject *parent, DB_functions_t *dbapi) : QObject(parent) {
    if (dbapi->plug_get_for_id ("artwork2")) {
            backend = new CoverArtNew(parent, dbapi);
    }
    else if (dbapi->plug_get_for_id("artwork")) {
        DB_plugin_t *p = dbapi->plug_get_for_id("artwork");
        if (p->api_vmajor == 1) {
            backend = new CoverArtLegacy(parent, dbapi);
        }
        else if (p->api_vmajor == 2) {
            backend = new CoverArtNew(parent,dbapi);
        }
    }
    if (backend && backend->getDefaultCoverArt()) {
        default_image = new QImage(backend->getDefaultCoverArt());
        cacheCoverArt(coverSearchValue(backend->getDefaultCoverArt()), default_image);
    }
}

CoverArtCache::~CoverArtCache() {
    if (default_image) {
        cacheUnref(default_image);
    }
    cmut_cache.lock();
    cmut_path.lock();
    cmut_refc.lock();
    QList<QImage *> l = cache_refc.keys();
    foreach(QImage *img, l) {
        if (cache_refc.value(img) == 0) {
            // reverse lookup :)
            coverSearch s;
            while (!(s = cache.key(img)).path.isEmpty()) {
                cache.remove(s);
                ddb_playItem_t *it;
                while ((it = cache_path.key(s.path))) {
                    cache_path.remove(it);
                }

            }
            delete img;
        }
        else {
            qDebug() << QString("Image (path: %1, size %2) has refc=%3!").arg(cache.key(img).path) .arg(cache.key(img).size.width()) .arg(cache_refc.value(img));
        }
    }
    cmut_cache.unlock();
    cmut_path.unlock();
    cmut_refc.unlock();
}

bool CoverArtCache::isCoverArtAvailable(DB_playItem_t *it, QSize size) {
    if (cache_path.contains(it)) {
        return cache.contains(coverSearchValue(cache_path.value(it),size));
    }
    return false;
}

QFuture<QImage *> CoverArtCache::requestCoverArt(DB_playItem_t *it, QSize size) {
    if (isCoverArtAvailable(it)) {
        // shouldn't be calling it
        return QtConcurrent::run(cover_art,cache.value(coverSearchValue(cache_path.value(it),size)));
    }
    return QtConcurrent::run(cover_art_load,this,it, size);
}

// Threaded static functions

QImage * CoverArtCache::cover_art(QImage *cover) {
    return cover;
}

QImage * CoverArtCache::cover_art_load(CoverArtCache *cac, DB_playItem_t *it, QSize size) {
    if (!it) {
        return nullptr;
    }
    QImage *ret = nullptr;
    // cache path for track
    if (cac->getCoverArtPath(it).isEmpty()) {
        QFuture<char *> c = cac->backend->loadCoverArt(it);
        c.waitForFinished();
        if (c.result() && strlen(c.result())) {
            cac->cachePath(it,c.result());
        }
        else {
            // no cover, use default
            cac->cachePath(it,"");
        }
    }
    // load full cover if missing
    bool unref_full_cover = false;
    if (!cac->isCoverArtAvailable(it,size) && !cac->isCoverArtAvailable(it)) {
        if (!cac->getCoverArtPath(it).isEmpty()) {
            QImage *img = new QImage(cac->getCoverArtPath(it));
            cac->cacheCoverArt(coverSearchValue(cac->getCoverArtPath(it),QSize()),img);
            ret = img;
            if (size.isValid()) {
                unref_full_cover = true;
            }
        }
        else {
            cac->cacheCoverArt(coverSearchValue(cac->getCoverArtPath(it),QSize()),nullptr);
        }
    }
    // scale cover if missing
    if (!cac->isCoverArtAvailable(it,size) && size.isValid() && !cac->getCoverArtPath(it).isEmpty()) {
        QImage *img = cac->getCoverArt(it);
        if (img) {
            QImage scaled = img->scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            QImage *n = new QImage(scaled);
            cac->cacheCoverArt(coverSearchValue(cac->getCoverArtPath(it),size),n);
            cac->cacheUnref(img);
            if (unref_full_cover) {
                cac->cacheUnref(img, true);
            }
            ret = n;
        }
    }
    return ret;
}

void CoverArtCache::cacheCoverArt(coverSearch s, QImage *img) {
    cmut_cache.lock();
    if (cache.contains(s)) {
        cmut_cache.unlock();
        qDebug() << "already cached?";
        return;
    }
    cache.insert(s,img);
    if (img) {
        cacheRef(img);
    }
    cmut_cache.unlock();
}

void CoverArtCache::cachePath(ddb_playItem_t *it, QString path) {
    cmut_path.lock();
    if (cache_path.contains(it)) {
        cache_path.remove(it);
    }
    cache_path.insert(it,path);
    cmut_path.unlock();
}

void CoverArtCache::cacheRef(QImage *img) {
    cmut_refc.lock();
    if (cache_refc.contains(img)) {
        int refc = cache_refc.take(img);
        cache_refc.insert(img, refc+1);
    }
    else {
        cache_refc.insert(img,1);
    }
    cmut_refc.unlock();
}

void CoverArtCache::cacheUnref(QImage *img, bool force_unref) {
    cmut_refc.lock();
    if (cache_refc.contains(img)) {
        // decrease
        int refc = cache_refc.take(img);
        cache_refc.insert(img, refc-1);
        // remove if cache full
        if ((cache_refc.size() > CACHE_SIZE || force_unref) && cache_refc.value(img) == 0) {
            cache_refc.remove(img);
            // reverse lookup :)
            coverSearch s;
            while (!(s = cache.key(img)).path.isEmpty()) {
                cache.remove(s);
                ddb_playItem_t *it;
                while ((it = cache_path.key(s.path))) {
                    cache_path.remove(it);
                }

            }
            delete img;
        }
    }
    cmut_refc.unlock();
}

void CoverArtCache::cacheUnrefTrack(DB_playItem_t *it) {
    if (cache_path.contains(it)) {
        // track no longer existent, no need to keep it in cache
        // cover arts are tracked separately
        cache_path.take(it);
    }
}

QImage * CoverArtCache::getCoverArt(DB_playItem_t *it, QSize size) {
    if (cache_path.contains(it) && cache.contains(coverSearchValue(cache_path.value(it),size))) {
        QImage *img = cache.value(coverSearchValue(cache_path.value(it),size));
        if (img) {
            cacheRef(img);
        }
        return img;
    }
    return nullptr;
}

QString CoverArtCache::getCoverArtPath(DB_playItem_t *it) {
    if (cache_path.contains(it)) {
        return cache_path.value(it);
    }
    return QString();
}


QImage * CoverArtCache::getCoverArtDefault() {
    if (!default_image && backend->getDefaultCoverArt()) {
        default_image = new QImage(backend->getDefaultCoverArt());
    }
    if (default_image) {
        cacheRef(default_image);
    }
    return default_image;
}
