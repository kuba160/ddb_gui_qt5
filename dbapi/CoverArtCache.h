#ifndef COVERARTCACHE_H
#define COVERARTCACHE_H

#include <QHash>
#include <QFutureWatcher>
#include "CoverArt.h"

class CoverArtCache : public ICoverArtCache {
    Q_OBJECT
public:
    CoverArtCache(QObject *parent, ICoverArtBackend *cover_provider);
    ~CoverArtCache();


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

signals:
    //void coverArtChanged(int type, int source_id, qint64 userdata, DB_playItem_t *it);

private:
    // cover backend
    ICoverArtBackend *backend = nullptr;

    // functions to be called from thread
    void registerNewPath(CoverArtRequest_t &, QString &);

    // cache ref/unref
    void cacheRef(QImage *img);
    void cacheUnref(QImage *img, bool force_unref = false);
    void cacheUnrefTrack(DB_playItem_t *it);

private:
    // Threaded static functions
    static QVariant cover_art_load(CoverArtCache *, CoverArtRequest_t *);
    static QImage *cover_art(QImage *cover);

    // Hashed covers (path and size links to QImage)
    //QHash<coverSearch, QImage *> cache;
    // a playitem links to path
    QHash<ddb_playItem_t *, QString> cache_path;
    QHash<QString, int> cache_refc;
    QSet<ddb_playItem_t *> m_processing;
    // QImage links to refc
    //QHash<QImage *, int> cache_refc;

    // mutex
    QMutex mut;
    //QMutex cmut_cache;
    //QMutex cmut_path;
};

#endif // COVERARTCACHE_H
