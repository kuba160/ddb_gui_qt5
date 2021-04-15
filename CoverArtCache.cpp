#include <QtConcurrent>

#include <unistd.h>
#include "CoverArtCache.h"
#include "DBApi.h"
#include "QtGui.h"

#undef DBAPI
#define DBAPI deadbeef_internal

#define CACHE_SIZE 100

extern DB_functions_t *deadbeef_internal;

CoverArtCache::CoverArtCache(QObject *parent) : QObject(parent) {
    artwork = static_cast<DB_artwork_plugin_t *>(static_cast<void *>(DBAPI->plug_get_for_id ("artwork")));
    if (artwork) {
        // force artwork to load default cover art
        getDefaultCoverArt();
    }
    // fix cover loading on startup
    QTimer::singleShot(1000,this, SLOT(refreshCoverArt()));

}

CoverArtCache::~CoverArtCache() {
    if (script_album_byte) {
        DBAPI->tf_free (script_album_byte);
        script_album_byte = nullptr;
    }
    if (script_artist_byte) {
        DBAPI->tf_free (script_artist_byte);
        script_artist_byte = nullptr;
    }
}

DB_artwork_plugin_t * CoverArtCache::getCoverArtPlugin () {
    return artwork;
}

void CoverArtCache::refreshCoverArt() {
    DB_playItem_t *it = DBAPI->streamer_get_playing_track();
    if (it) {
        currCover.setFuture(loadCoverArt(it));
        DBAPI->pl_item_unref(it);
    }
    else {
        currCover.setFuture(loadCoverArt(nullptr,nullptr,nullptr));
    }
}

QImage *CoverArtCache::getDefaultCoverArt() {
    if (!default_image) {
        const char * def_cover = getCoverArtPlugin()->get_default_cover();
        if (def_cover) {
            default_image = new QImage(getCoverArtPlugin()->get_default_cover());
        }
    }
    return default_image;
}

CoverArtCache *cac_current = nullptr;

void cover_avail_callback(const char *fname, const char *artist, const char *album, void *user_data) {
    if (!user_data) {
        qDebug() << "cover_avail_callback: no user_data!" << ENDL;
        return;
    }
    CoverArtCache *cac = static_cast<CoverArtCache *>(user_data);
    if (!fname && !artist && !album) {
        // well, artwork_legacy calls this when no cover is found
        if (!cac->cache.value(cac->album_lookup))
            cac->cache.insert(cac->album_lookup,cac->getDefaultCoverArt());
        return;
    }

    DB_artwork_plugin_t *artwork = cac->getCoverArtPlugin();
    char *image_fname = artwork->get_album_art(fname, artist, album, -1, (&cover_avail_callback), cac);
    if (image_fname) {
        // add to cac cache
        qDebug() << "cover_avail_callback: adding" << album << "to cache.";
        cac->cacheSizeCheck();
        cac->cache.insert(album,new QImage(image_fname));
    }
    else {
        // no image / add default
        // note: this code will probably never be run
        cac->cacheSizeCheck();
        cac->cache.insert(album, cac->getDefaultCoverArt());
    }
    return;
}

QImage * thread_getCoverArt(QString fname, QString artist, QString album) {
    CoverArtCache *cac = cac_current;
    DB_artwork_plugin_t *artwork = cac->getCoverArtPlugin();

    if (fname.isNull() && artist.isNull() && album.isNull()) {
        return cac->getDefaultCoverArt();
    }

    char *image_fname = artwork->get_album_art(fname.toUtf8(), artist.toUtf8(), album.toUtf8(), -1, (&cover_avail_callback), cac);
    if (!image_fname) {
        // wait
        usleep(100000);
        while (!cac->cache.value(album)) {
            // TODO, what if callback gets never called? :(
            usleep(500000);
        }
    }
    else {
        if (!cac->cache.value(album)) {
            // cached by artwork, but not by us?
            // caching anyway
            qDebug() << "thread_getCoverArt: adding" << album << "to cache.";
            cac->cacheSizeCheck();
            cac->cache.insert(album,new QImage(image_fname));
        }
    }
    return cac->cache.value(album);
}


QFuture<QImage *> CoverArtCache::loadCoverArt(DB_playItem_t *track) {
    if (!artwork) {
        return QFuture<QImage*>();
    }
    ddb_tf_context_t *context = new ddb_tf_context_t;
    context->_size = sizeof(ddb_tf_context_t);
    context->it = track;
    context->iter = PL_MAIN;

    if (!script_album_byte) {
        script_album_byte = DBAPI->tf_compile("%album%");
    }
    if (!script_artist_byte) {
        script_artist_byte = DBAPI->tf_compile("%artist%");
    }

    // TODO adjust length maybe
    char fname[256];
    char artist[256];
    char album[256];
    if (track) {
        DBAPI->pl_lock();
        strncpy(fname, DBAPI->pl_find_meta(track, ":URI"), 256);
        DBAPI->pl_unlock();
        DBAPI->tf_eval (context, script_artist_byte, artist, 256);
        DBAPI->tf_eval (context, script_album_byte, album, 256);
    }
    delete context;
    cac_current = this;
    strcpy(album_lookup,album);
    return QtConcurrent::run(thread_getCoverArt,QString(fname),QString(artist),QString(album));
}


QFuture<QImage *> CoverArtCache::loadCoverArt(const char *fname, const char *artist, const char *album) {
    if (!artwork) {
        return QFuture<QImage*>();
    }
    cac_current = this;
    if (album)
        strcpy(album_lookup,album);
    return QtConcurrent::run(thread_getCoverArt,fname,artist,album);
}

void CoverArtCache::cacheClear() {
    cache.clear();
}

void CoverArtCache::cacheSizeCheck() {
    if (cache.size() >= CACHE_SIZE) {
        // hope that constBegin is the oldest one...
        cache.erase(cache.constBegin());
    }
}

void CoverArtCache::removeCoverArt(const char *album) {
    delete cache.value(album);
    cache.remove(album);
}

void CoverArtCache::trackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    if (from != to && to) {
        currCover.setFuture(loadCoverArt(to));
    }
    else {
        // set empty cover
        currCover.setFuture(loadCoverArt(nullptr,nullptr,nullptr));
    }
}
