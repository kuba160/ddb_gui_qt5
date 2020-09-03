#include "CoverArtCache.h"
#include "DBApi.h"

#define CACHE_SIZE 30

extern DB_functions_t *deadbeef_internal;

CoverArtCache::CoverArtCache(QObject *parent) : QObject(parent) {
    //connect(CoverArtWrapper::Instance(), SIGNAL(coverIsReady(const QImage &)), this, SLOT(putCover(const QImage &)));
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
    // todo add fname and artist
}

DB_artwork_plugin_t * CoverArtCache::getCoverArtPlugin () {
    // todo its possible that artwork plugin is missing, no checks done for that...
    return static_cast<DB_artwork_plugin_t *>(static_cast<void *>(DBAPI->plug_get_for_id ("artwork")));
}

void CoverArtCache::cacheSizeCheck() {
    if (cache.size() >= CACHE_SIZE) {
        // hope that constBegin is the oldest one...
        cache.erase(cache.constBegin());
    }
}

QImage *CoverArtCache::getDefaultCoverArt() {
    if (!default_image) {
        default_image = new QImage(getCoverArtPlugin()->get_default_cover());
    }
    return default_image;
}

CoverArtCache *cac_current = nullptr;

void cover_avail_callback(const char *fname, const char *artist, const char *album, void *user_data) {
    if ((!fname && !artist && !album) || !user_data) {
        // well, artwork_legacy calls this when no cover is found, but does not pass user_data :(
        if (cac_current) {
            // no info about album, so what now? :(
            if (!cac_current->default_image) {
                cac_current->default_image = new QImage(cac_current->getCoverArtPlugin()->get_default_cover());
            }
            cac_current->makeEmitNewCoverSignal(cac_current->current_handle, cac_current->default_image);
            cac_current->current_handle = 0;
        }
        return;
    }
    CoverArtCacheRequest_t *cacr = static_cast<CoverArtCacheRequest_t *>(user_data);
    DB_artwork_plugin_t *artwork = cacr->cache->getCoverArtPlugin();
    char *image_fname = artwork->get_album_art(fname, artist, album, -1, (&cover_avail_callback), NULL);
    if (image_fname) {
        // add to cac cache
        qDebug() << "cover_avail_callback: adding" << album << "to cache.";
        cacr->cache->cacheSizeCheck();
        cacr->cache->cache.insert(album,new QImage(image_fname));
    }
    else {
        // no image / add default
        // note: this code will probably never be run
        cacr->cache->cacheSizeCheck();
        cacr->cache->cache.insert(album, new QImage(artwork->get_default_cover()));
    }

    cacr->cache->makeEmitNewCoverSignal(cacr->requestnum, cacr->cache->cache.value(album));
    delete cacr;
}

void CoverArtCache::makeEmitNewCoverSignal(uint32_t num, QImage *img) {
    qRegisterMetaType < uint32_t >("uint32_t");
    emit coverLoaded(num, img);
}

void CoverArtCache::loadCoverArt(DB_playItem_t *track, uint32_t handle_num) {
    ddb_tf_context_t context;
    context._size = sizeof(ddb_tf_context_t);
    context.flags = 0;
    context.it = track;
    context.idx = 0;
    context.id = 0;
    context.iter = PL_MAIN;
    context.update = 0;
    context.dimmed = 0;

    if (!script_album_byte) {
        script_album_byte = DBAPI->tf_compile("%album%");
    }
    if (!script_artist_byte) {
        script_artist_byte = DBAPI->tf_compile("%artist%");
    }

    char fname[256];
    char artist[256];
    char album[256];
    DBAPI->pl_lock();
    strncpy(fname, DBAPI->pl_find_meta(track, ":URI"), 256);
    DBAPI->pl_unlock();
    DBAPI->tf_eval (&context, script_artist_byte, artist, 256);
    DBAPI->tf_eval (&context, script_album_byte, album, 256);

    return loadCoverArt(fname, artist, album, handle_num);
}

void CoverArtCache::loadCoverArt(const char *fname, const char *artist, const char *album, uint32_t handle_num) {
    // todo fname/artist when no album name?
    //currentName = QString::fromUtf8(album);


    if (cache.contains(album)) {
        emit coverLoaded(handle_num ? handle_num : rand_gen.generate(),cache.value(album));
        return;
        //emit coverIsReady(cache.value(currentName));
    }
    else {
        CoverArtCacheRequest_t *request = new CoverArtCacheRequest_t;
        if (handle_num) {
            request->requestnum = handle_num;
        }
        else {
            request->requestnum = rand_gen.generate();
        }
        request->cache = this;
        cac_current = this;
        current_handle = request->requestnum;
        char *image_fname = getCoverArtPlugin()->get_album_art(fname, artist, album, -1, (&cover_avail_callback), request);
        if (image_fname) {
            // add to cac cache
            cacheSizeCheck();
            cache.insert(album,new QImage(image_fname));
            emit coverLoaded(request->requestnum, cache.value(album));
            delete request;
        }
        else {
            // no image or it probably needs to get loaded, wait for callback
        }
    }
}

void CoverArtCache::cacheClear() {
    cache.clear();
}


void CoverArtCache::removeCoverArt(const char *album) {
    delete cache.value(album);
    cache.remove(album);
}

void CoverArtCache::currCoverChangeCheck(DB_playItem_t *track_from, DB_playItem_t *track_to) {
    // change to NULL if playback stopped
    if (track_to == nullptr) {
        //emit currCoverChanged(nullptr);
        return;
    }

    // do not change if same album
    ddb_tf_context_t context;
    context._size = sizeof(ddb_tf_context_t);
    context.flags = 0;
    context.it = track_from;
    context.idx = 0;
    context.id = 0;
    context.iter = PL_MAIN;
    context.update = 0;
    context.dimmed = 0;
    if (!script_album_byte) {
        script_album_byte = DBAPI->tf_compile("%album%");
    }

    char album_1[256];
    char album_2[256];

    DBAPI->tf_eval (&context, script_album_byte, album_1, 256);
    context.it = track_to;
    DBAPI->tf_eval (&context, script_album_byte, album_2, 256);

    if (strcmp(album_1,album_2) == 0) {
        return;
    }

    // 2 diff albums, load new ones
    uint32_t handle = rand_gen.generate();
    emit currCoverChanged(handle);
    loadCoverArt(track_to, handle);
    return;
}

/*
void CoverArtCache::getDefaultCoverArt() {
    if (!COVERART) {
        return;
    }
    const char *image_fname = COVERART->get_default_cover();
    if (image_fname) {
        coverLoadWatcher.setFuture(QtConcurrent::run(scale, image_fname));
    }
}
*/

/*
void CoverArtCache::openAndScaleCover(const char *fname) {
    coverLoadWatcher.setFuture(QtConcurrent::run(scale, fname));
}


QImage *scale(const char *fname) {
    QImage *pm = new QImage(QString::fromUtf8(fname));
    if (pm->isNull()) {
        qDebug() << "Unsupported image format";
        delete pm;
        pm = new QImage(QString::fromUtf8(COVERART->get_default_cover()));
    }
    QImage *scaledImage = new QImage(pm->scaled(CoverArtWrapper::Instance()->defaultWidth, CoverArtWrapper::Instance()->defaultWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    delete pm;
    return scaledImage;
}**/


