#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QHash>
#include <QFutureWatcher>
#include "include/artwork.h"

class CoverArtCache : public QObject {
    Q_OBJECT

public:
    CoverArtCache(QObject *parent = nullptr);
    ~CoverArtCache();

    // Get Cover Art plugin
    inline DB_artwork_plugin_t *getCoverArtPlugin ();

    // QFuture pointer for cover loading
    QFuture<QImage *> loadCoverArt(const char *fname, const char *artist, const char *album);
    QFuture<QImage *> loadCoverArt(DB_playItem_t *);

    // QFutureWatcher for watching for current cover art
    inline QFutureWatcher<QImage *> getCurrentCoverWatcher();

    QImage *getDefaultCoverArt();


    // checks if cache reached predefined size, free oldest one if necessary to allow 1 more write
    void cacheSizeCheck();
    // empty the cache
    void cacheClear();
    //
    void removeCoverArt(const char *album);

    // Hashed covers
    QHash<QString, QImage *> cache;
    // currCover for subscription
    QFutureWatcher<QImage *> currCover;


    char album_lookup[255];

private:
    DB_artwork_plugin_t *artwork;
    QImage *default_image = nullptr;
    char *script_album_byte = nullptr;
    char *script_artist_byte = nullptr;

public slots:
    // cache cover on song change
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
};

#endif // COVERARTCACHE_H
