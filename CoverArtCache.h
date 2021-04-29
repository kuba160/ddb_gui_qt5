#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QHash>
#include <QFutureWatcher>
#include <CoverArtBackend.h>

#include <deadbeef/deadbeef.h>

#define COVERARTCACHE_P(X) (static_cast<CoverArtCache *>(X))

// used internally to find scaled cover
typedef struct scaledCover_s {
    void *img;
    QSize size;
} scaledCover;

class CoverArtCache : public QObject {
    Q_OBJECT

public:
    CoverArtCache(QObject *parent = nullptr, DB_functions_t *funcs = nullptr);
    ~CoverArtCache();

    // check if track already cached
    bool isCoverArtAvailable(DB_playItem_t *);
    // QFuture pointer for cover loading
    QFuture<QImage *> requestCoverArt(DB_playItem_t *);
    // load cached cover art
    QImage * getCoverArt(DB_playItem_t *);
    // load cached cover art, helper used when found that cover art is already cached
    QImage * getCoverArt(QString path);
    // scale coverart and cache it
    QImage * getCoverArtScaled(QImage *img, QSize size);
    // get default cover art
    QImage * getCoverArtDefault();

    // backend, either CoverArtLegacy or CoverArtNew (artwork2)
    CoverArtBackend *backend = nullptr;
    QImage *default_image = nullptr;

    // functions to be called from thread
    void cacheCoverArt(DB_playItem_t *it, QImage *img);
    void cacheCoverArt(QString path, QImage *img);

    // cache ref/unref
    void cacheRef(QImage *img);
    void cacheUnref(QImage *img);
    void cacheUnrefTrack(DB_playItem_t *it);

protected:
    static QImage * cover_art_load(CoverArtCache *, DB_playItem_t *);
    static QImage *cover_art(QImage *cover);

    // Hashed covers
    QHash<DB_playItem_t *, QImage *> cache;
    QHash<QString, QImage *> cache_path;
    QHash<QImage *, int> cache_refc;
    QHash<scaledCover, QImage *> cache_scaled;

    // mutex
    QMutex cmut;
    QMutex cmut_refc;
    // function access
    DB_functions_t *db;
};

#endif // COVERARTCACHE_H
