#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QHash>
#include <QFutureWatcher>
#include <CoverArtBackend.h>

#include <deadbeef/deadbeef.h>

#define COVERARTCACHE_P(X) (static_cast<CoverArtCache *>(X))

typedef struct coverSearch_s {
    QString path;
    QSize size;
} coverSearch;

class CoverArtCache : public QObject {
    Q_OBJECT

public:
    CoverArtCache(QObject *parent = nullptr, DB_functions_t *funcs = nullptr);
    ~CoverArtCache();

    // check if track already cached (full size or specific size)
    bool isCoverArtAvailable(DB_playItem_t *it, QSize size = QSize());
    // QFuture pointer for cover loading
    QFuture<QImage *> requestCoverArt(DB_playItem_t *, QSize size);
    // load cached cover art
    QImage * getCoverArt(DB_playItem_t *, QSize size = QSize());
    // get coverart path
    QString getCoverArtPath(DB_playItem_t *it);
    // get default cover art
    QImage * getCoverArtDefault();

    // backend, either CoverArtLegacy or CoverArtNew (artwork2)
    CoverArtBackend *backend = nullptr;
    QImage *default_image = nullptr;

    // functions to be called from thread
    void cacheCoverArt(coverSearch, QImage *img);
    void cachePath(ddb_playItem_t *it, QString);

    // cache ref/unref
    void cacheRef(QImage *img);
    void cacheUnref(QImage *img, bool force_unref = false);
    void cacheUnrefTrack(DB_playItem_t *it);

    static coverSearch coverSearchValue(QString path, QSize size = QSize()) {
        coverSearch s;
        s.path = path;
        s. size = size;
        return s;
    }

protected:
    // Threaded static functions
    static QImage *cover_art_load(CoverArtCache *, DB_playItem_t *, QSize size);
    static QImage *cover_art(QImage *cover);

    // Hashed covers (path and size links to QImage)
    QHash<coverSearch, QImage *> cache;
    // a playitem links to path
    QHash<ddb_playItem_t *, QString> cache_path;
    // QImage links to refc
    QHash<QImage *, int> cache_refc;

    // mutex
    QMutex cmut_refc;
    QMutex cmut_cache;
    QMutex cmut_path;
};

#endif // COVERARTCACHE_H
