#include "CoverArtCache.h"

#include "CoverArtWrapper.h"

CoverArtCache *CoverArtCache::instance;

void CoverArtCache::Destroy() {
    CoverArtWrapper::Instance()->Destroy();
    delete instance;
    instance = NULL;
}

CoverArtCache *CoverArtCache::Instance(QObject *parent) {
    if (instance == NULL) {
        instance = new CoverArtCache(parent);
    }
    return instance;
}

CoverArtCache::CoverArtCache(QObject *parent) {
    connect(CoverArtWrapper::Instance(), SIGNAL(coverIsReady(const QImage &)), this, SLOT(putCover(const QImage &)));
}

void CoverArtCache::getCoverArt(const char *fname, const char *artist, const char *album) {
    currentName = QString::fromUtf8(album);
    if (cache.contains(currentName))
        emit coverIsReady(cache.value(currentName));
    else {
        CoverArtWrapper::Instance()->getCoverArt(fname, artist, album);
    }
}

void CoverArtCache::getDefaultCoverArt() {
    CoverArtWrapper::Instance()->getDefaultCoverArt();
}


void CoverArtCache::putCover(const QImage &cover) {
    if (cache.size() == CACHE_SIZE)
        cache.remove(cache.keys().first());
    cache.insert(currentName, cover);
    emit coverIsReady(cover);
}

void CoverArtCache::removeCoverArt(const char *artist, const char *album) {
    cache.remove(QString::fromUtf8(album));
}

void CoverArtCache::clear() {
    cache.clear();
}
