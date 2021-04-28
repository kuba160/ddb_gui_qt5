#include <QtConcurrent>

#include <unistd.h>
#include "CoverArtCache.h"
#include "DBApi.h"
#include "QtGui.h"

#undef DBAPI
#define DBAPI deadbeef_internal

#define CACHE_SIZE 100

extern DB_functions_t *deadbeef_internal;

#undef DBAPI
#define DBAPI db

bool operator==(const scaledCover &lhs, const scaledCover &rhs) noexcept {
    return lhs.img == rhs.img && lhs.size == rhs.size;
}

uint qHash(const scaledCover &c, uint seed) noexcept {
    QtPrivate::QHashCombine hash;
    seed = hash(seed, c.img);
    seed = hash(seed, c.size.height());
    seed = hash(seed, c.size.width());
    return seed;
}

CoverArtCache::CoverArtCache(QObject *parent, DB_functions_t *funcs) : QObject(parent) {
    db = funcs;
    if(DBAPI->plug_get_for_id ("artwork")) {
        backend = new CoverArtLegacy(parent, db);
    }
    else if(DBAPI->plug_get_for_id ("artwork2")) {
        backend = new CoverArtNew(parent, db);
    }

    default_image = new QImage(backend->getDefaultCoverArt());
    cacheCoverArt(backend->getDefaultCoverArt(), default_image);
    cacheRef(default_image);
}

CoverArtCache::~CoverArtCache() {
    if (default_image) {
        cacheUnref(default_image);
    }
    // TODO clean up cache
    QList<QImage *> l = cache_refc.keys();
    foreach(QImage *img, l) {
        if (cache_refc.value(img) == 0) {
            delete img;
            QList<scaledCover> l_sc = cache_scaled.keys();
            foreach (scaledCover sc, l_sc) {
                if (sc.img == img) {
                    delete cache_scaled.value(sc);
                }
            }
        }
        else {
            qDebug() << QString("Image (path: %1) has refc=%2!").arg(cache_path.key(img)) .arg(cache_refc.value(img));
        }
    }
}

bool CoverArtCache::isCoverArtAvailable(DB_playItem_t *it) {
    return cache.contains(it);
}

QFuture<QImage *> CoverArtCache::requestCoverArt(DB_playItem_t *it) {
    if (isCoverArtAvailable(it)) {
        // shouldn't be calling it
        return QtConcurrent::run(cover_art,cache.value(it));
    }
    return QtConcurrent::run(cover_art_load,this,it);
}

// Threaded static functions

QImage * CoverArtCache::cover_art(QImage *cover) {
    return cover;
}

QImage * CoverArtCache::cover_art_load(CoverArtCache *cac, DB_playItem_t *it) {
    if (!it) {
        return nullptr;
    }
    QFuture<char *> c = cac->backend->loadCoverArt(it);
    c.waitForFinished();
    if (c.result() && strlen(c.result())) {
        // check if path already cached
        cac->cmut.lock();
        QImage *img = cac->getCoverArt(c.result());
        if (img) {
            cac->cmut.unlock();
            return img;
        }
        // result not cached, create new and cache
        img = new QImage(c.result());
        if (img) {
            cac->cacheCoverArt(it,img);
            cac->cacheCoverArt(c.result(),img);
            cac->backend->unloadCoverArt(c.result());
            cac->cmut.unlock();
            return img;
        }
        else {
            cac->cmut.unlock();
            qDebug() << "loading image " << c.result() << " failed!" << ENDL;
        }
    }
    return nullptr;
}

void CoverArtCache::cacheCoverArt(DB_playItem_t *it, QImage *img) {
    if (cache.contains(it)) {
        qDebug() << "already cached?";
    }
    cache.insert(it,img);
    cacheRef(img);
}

void CoverArtCache::cacheCoverArt(QString path, QImage *img) {
    cache_path.insert(path,img);
}

void CoverArtCache::cacheRef(QImage *img) {
    if (cache_refc.contains(img)) {
        int refc = cache_refc.take(img);
        cache_refc.insert(img, refc+1);
    }
    else {
        cache_refc.insert(img,1);
    }
}

void CoverArtCache::cacheUnref(QImage *img) {
    if (cache_refc.contains(img)) {
        // decrease
        int refc = cache_refc.take(img);
        cache_refc.insert(img, refc-1);
        // remove if cache full
        if (cache_refc.size() > CACHE_SIZE && cache_refc.value(img) == 0) {
            qDebug() << "no more refc on image, deleting" << ENDL;
            // cache
            DB_playItem_t *it = cache.key(img);
            if (it) {
                cache.take(it);
            }
            QString s = cache_path.key(img);
            if (!s.isEmpty()) {
                cache_path.take(s);
            }
            QList<scaledCover> l_sc = cache_scaled.keys();
            foreach (scaledCover sc, l_sc) {
                if (sc.img == img) {
                    QImage *i = cache_scaled.take(sc);
                    delete i;
                }
            }
            cache_refc.take(img);
            delete img;
        }
    }
}

QImage * CoverArtCache::getCoverArt(DB_playItem_t *it) {
    if (cache.contains(it)) {
        QImage *img = cache.value(it);
        cacheRef(img);
        return img;
    }
    return nullptr;
}

QImage * CoverArtCache::getCoverArt(QString path) {
    if (cache_path.contains(path)) {
        QImage *img = cache_path.value(path);
        cacheRef(img);
        return img;
    }
    return nullptr;
}

QImage * CoverArtCache::getCoverArtScaled(QImage *img, QSize size) {
    scaledCover c = {img,size};
    if (cache_scaled.contains(c)) {
        return cache_scaled.value(c);
    }
    // create new one
    QImage scaled = img->scaled(size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    QImage *n = new QImage(scaled);
    cache_scaled.insert(c,n);
    return n;
}

QImage * CoverArtCache::getCoverArtDefault() {
    if (default_image) {
        cacheRef(default_image);
    }
    return default_image;
}
