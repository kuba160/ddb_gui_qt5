#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QHash>
#include <QFutureWatcher>
#include <QRandomGenerator>
#include "include/artwork.h"

class CoverArtCache : public QObject {
    Q_OBJECT

public:
    CoverArtCache(QObject *parent = nullptr);
    ~CoverArtCache();

    DB_artwork_plugin_t *getCoverArtPlugin ();

    // load cover art into database
    void loadCoverArt(const char *fname, const char *artist, const char *album, uint32_t handle_num = 0);
    void loadCoverArt(DB_playItem_t *, uint32_t handle_num = 0);

    QImage *getDefaultCoverArt();
    // checks if cache reached predefined size, free oldest one if necessary to allow 1 more write
    void cacheSizeCheck();
    // empty the cache
    void cacheClear();
    //
    void removeCoverArt(const char *album);

    void makeEmitNewCoverSignal(uint32_t, QImage *);

    QHash<QString, QImage *> cache;

    uint32_t current_handle;
    QImage *default_image = nullptr;
    //QString currentName;
    //QFutureWatcher<QImage *> coverLoadWatcher;
    // void removeCoverArt(const char *album);
    char *script_album_byte = nullptr;
    char *script_artist_byte = nullptr;

private:
    QRandomGenerator rand_gen;
public slots:
    // links with trackChanged/DBApi
    void currCoverChangeCheck(DB_playItem_t *, DB_playItem_t *);
    // old
    // void putCover(const QImage &);

signals:
    //
    void coverLoaded(uint32_t request, QImage *image);

    //
    void currCoverChanged (uint32_t request);
    // old
    void coverIsReady(const QImage &);
};

typedef struct CoverArtCacheRequest_s {
    uint32_t requestnum;
    CoverArtCache *cache;
} CoverArtCacheRequest_t;

#endif // COVERARTCACHE_H
