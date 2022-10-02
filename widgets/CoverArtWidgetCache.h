#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QImage>
#include <QHash>
#include <QFutureWatcher>

#include <dbapi/DBApi.h>
#include <deadbeef/deadbeef.h>

typedef struct ScaledCoverEntry_s {
    QString path;
    QSize size;
} ScaledCoverEntry_t;

class CoverArtWidgetCache : public ICoverArtCache {
    Q_OBJECT
    DBApi *api;
public:
    CoverArtWidgetCache(QObject *parent, DBApi *Api);
    ~CoverArtWidgetCache();


    // check if coverart is in cache, returns CoverArtStatusFlags
    virtual CoverArtStatusFlags coverArtStatus(CoverArtRequest_t &) override;
    // load cover that is not in cache, QImage returned via QFuture has to be unref'd later
    // if you set up size, the cover will be scaled
    // use scaling if you need many covers of specific size, don't use for widget scaling etc.
    QFuture<QVariant> requestCoverArt(CoverArtRequest_t &) override;
    // get cached cover art, nullptr if not cached (use requestCoverArt to cache it)
    QVariant getCoverArt(CoverArtRequest_t &) override;
    // Call after you are done with cover
    void unref(CoverArtType type, QVariant) ;//override;
    // Increase reference count if needed
    void ref(CoverArtType type, QVariant) ;//override;
    // Call if track becomes no longer accessible to clear cache (does not remove currently used items)
    void track_unref(DB_playItem_t *) ;//override;


    QImage* cover_art_cache_scaled(QString &path, QSize size);

    CoverArtStatusFlags coverArtPathAvailable(CoverArtRequest_t *);
    QVariant getCoverArtPath(CoverArtRequest_t *);
    QFuture<QVariant> requestCoverArtPath(CoverArtRequest_t *);

signals:
    //void coverArtChanged(int type, int source_id, qint64 userdata, DB_playItem_t *it);

private:
    // functions to be called from thread
    void registerNewCover(CoverArtRequest_t *, QString &path, QImage *);

    // cache ref/unref
    void cacheRef(QImage *img);
    void cacheUnref(QImage *img, bool force_unref = false);
    void cacheUnrefTrack(DB_playItem_t *it);

private:
    // Threaded static functions
    static QVariant cover_art_load(DBApi *api, CoverArtWidgetCache *, CoverArtRequest_t *);





    // Hashed covers (path and size links to QImage)
    //QHash<coverSearch, QImage *> cache;
    // a playitem links to path
    QHash<QString, QImage *> cache_img;
    QHash<ScaledCoverEntry_t, QImage *> cache_img_scaled;
    QHash<QString, int> cache_refc;
    QSet<CoverArtRequest_t *> m_processing;
    // QImage links to refc
    //QHash<QImage *, int> cache_refc;

    // mutex
    QMutex mut;

};


#endif // COVERARTCACHE_H
