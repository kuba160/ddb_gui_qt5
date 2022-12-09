#ifndef COVERART_H
#define COVERART_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QSize>
#include <QHash>

#include <deadbeef/deadbeef.h>

// abstract CoverArtBackend definition
class ICoverArtBackend : public QObject {
    Q_OBJECT
public:
    explicit ICoverArtBackend(QObject *parent) : QObject(parent) {};
    virtual QFuture<char *> loadCoverArt(DB_playItem_t *) = 0;
    virtual const char * getDefaultCoverArt() = 0;
    virtual void unloadCoverArt(const char*) = 0;
};

enum CoverArtStatusFlags {
    STATUS_UNAVAIL = 0, // Impossible to process that request (type not supported)
    STATUS_MISS = 1 << 0, // Cover not in cache
    STATUS_CACHED = 1 << 1, // Cover cached and available
    STATUS_LOADING = 1 << 2, // Cover caching but not yet available
    STATUS_PARTIAL = 1 << 3 // Cover cached but not available at given size
};
//Q_ENUMS(CoverArtStatusFlags)
enum CoverArtType {
    COVER_QSTRING = 1 << 1, //
    COVER_QIMAGE = 1 << 2
};

class ICoverArtCache : public QObject {
    Q_OBJECT
public:
    ICoverArtCache(QObject *parent) : QObject(parent) {};

    typedef struct CoverArtRequest_s {
        DB_playItem_t *it;
        QSize size; // not supported for COVER_QSTRING
        CoverArtType type;
        qint64 source_id; // requester ID
        qint64 userdata;
    } CoverArtRequest_t;

    // check if coverart is in cache, returns CoverArtStatusFlags
    virtual CoverArtStatusFlags coverArtStatus(CoverArtRequest_t &) = 0;
    // load cover that is not in cache, QImage returned via QFuture has to be unref'd later
    // if you set up size, the cover will be scaled
    // use scaling if you need many covers of specific size, don't use for widget scaling etc.
    virtual QFuture<QVariant> requestCoverArt(CoverArtRequest_t &) = 0;
    // get cached cover art, nullptr if not cached (will be cached automatically)
    virtual QVariant getCoverArt(CoverArtRequest_t &) = 0;
    // Call after you are done with cover
    //virtual void unref(CoverArtType, QVariant) = 0;
    // Increase reference count if needed
    //virtual void ref(CoverArtType, QVariant) = 0;
    // Call if track becomes no longer accessible to clear cache (does not remove currently used items)
    //virtual void track_unref(DB_playItem_t *) = 0;

signals:
    void coverArtChanged(int type, int source_id, qint64 userdata, DB_playItem_t *it);
};

class CoverArt : public ICoverArtCache
{
    Q_OBJECT
    DB_functions_t *deadbeef;
    ICoverArtBackend *cover_provider = nullptr;
    QHash<int, ICoverArtCache *> cache_hash;

    Q_PROPERTY(ICoverArtBackend * cover_provider MEMBER cover_provider CONSTANT)

public:
    explicit CoverArt(QObject *parent, DB_functions_t *api);
    ~CoverArt();

    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);

    // ensure cover art plugin is available before using any functions below
    Q_PROPERTY(bool available READ getCoverArtAvailable CONSTANT)
    //Q_PROPERTY(bool cacheAvailable READ getCoverArtCacheAvailable CONSTANT)

    //QFuture<char *> getCoverArtPath(DB_playItem_t *);

    Q_PROPERTY(QUrl current_cover_path READ getCurrentCoverPath NOTIFY currentCoverPathChanged)
    QUrl getCurrentCoverPath();

    // inserts cover cache type and aquires its ownership
    bool insertCoverArtCache(int type, ICoverArtCache *obj);
    qint64 allocateSourceId();
    //
    // check if coverart is in cache, returns CoverArtStatusFlags
    CoverArtStatusFlags coverArtStatus(CoverArtRequest_t &) override;
    // load cover that is not in cache, QImage returned via QFuture has to be unref'd later
    // if you set up size, the cover will be scaled
    // use scaling if you need many covers of specific size, don't use for dynamic widget scaling etc.
    QFuture<QVariant> requestCoverArt(CoverArtRequest_t &) override;
    // get cached cover art, nullptr if not cached (will be cached automatically)
    QVariant getCoverArt(CoverArtRequest_t &) override;


public slots:
    bool getCoverArtAvailable();

signals:
    void currentCoverPathChanged();

private:
    QFutureWatcher<char *> current_cover_path_watcher;
    QString m_current_cover_path;
    qint64 source_id_next = 1;

private slots:
    void current_cover_path_handler();

};

#endif // COVERART_H
