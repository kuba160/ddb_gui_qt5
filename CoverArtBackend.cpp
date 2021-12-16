#include <QtConcurrent>
#include "CoverArtBackend.h"

#undef DBAPI
#define DBAPI db

// CoverArtNew

CoverArtNew::CoverArtNew(QObject *parent, DB_functions_t *funcs) : CoverArtBackend(parent, funcs) {
    plug = DBAPI->plug_get_for_id("artwork2") ?
            static_cast<ddb_artwork_plugin_t *>(static_cast<void *>(DBAPI->plug_get_for_id("artwork2"))):
            static_cast<ddb_artwork_plugin_t *>(static_cast<void *>(DBAPI->plug_get_for_id("artwork")));
    if (!plug) {
        qDebug() << "failed to load artwork!" << ENDL;
    }
}

QFuture<char *> CoverArtNew::loadCoverArt(DB_playItem_t *it) {
    return QtConcurrent::run(getArtwork,it,this);
}

char * CoverArtNew::getArtwork(DB_playItem_t *it, CoverArtNew *can) {
    ddb_cover_query_t *query = new ddb_cover_query_t;
    ddb_cover_info_t *cover_info = nullptr;
    query->_size = sizeof(ddb_cover_query_t);
    query->user_data = &cover_info;
    query->track = it;

    can->plug->cover_get(query,artwork_callback);
    while (!cover_info) {
        // hope we don't get stuck in endless loop...
        QThread::msleep(100);
    }

    if (!cover_info->cover_found) {
        if (!cover_info->refc) {
            delete cover_info;
        }
        else {
            can->plug->cover_info_free(cover_info);
        }
        return nullptr;
    }
    can->ht.insert(cover_info->image_filename,cover_info);
    return cover_info->image_filename;
}

void CoverArtNew::artwork_callback(int error, ddb_cover_query_t *query, ddb_cover_info_t *cover) {
    if (error) {
        // failed, create dummy ddb_cover_info_t and return
        cover = (ddb_cover_info_t *) malloc(sizeof(ddb_cover_info_t));
        cover->type = nullptr;
        cover->image_filename = nullptr;
        cover->blob = nullptr;
        cover->refc = 0;
    }
    *(static_cast<ddb_cover_info_t **>(query->user_data)) = cover;
    delete query;
    return;
}

void CoverArtNew::unloadCoverArt(const char *path) {
    if (ht.contains(path)) {
        ddb_cover_info_t *ci = ht.take(path);
        plug->cover_info_free(ci);
    }
}

const char *CoverArtNew::getDefaultCoverArt() {
    // todo
    return nullptr;
}

// CoverArtLegacy

CoverArtLegacy::CoverArtLegacy(QObject *parent, DB_functions_t *funcs) : CoverArtBackend(parent, funcs) {
    //
    plug = static_cast<DB_artwork_plugin_t *>(static_cast<void *>(DBAPI->plug_get_for_id ("artwork")));
    if (!plug) {
        qDebug() << "failed to load artwork!" << ENDL;
        return;
    }
    script_album_byte = DBAPI->tf_compile("%album%");
    script_artist_byte = DBAPI->tf_compile("%artist%");
    // fix default coverart not loading
    plug->get_default_cover();
}

CoverArtLegacy::~CoverArtLegacy() {
    if (script_album_byte) {
        DBAPI->tf_free(script_album_byte);
    }
    if (script_artist_byte) {
        DBAPI->tf_free(script_artist_byte);
    }
}

QFuture<char*> CoverArtLegacy::loadCoverArt(DB_playItem_t *it) {
    if (!it) {
        return QtConcurrent::run(getArtwork,nullptr,nullptr,nullptr, plug);
    }
    // unable to find cover by item in artwork-legacy, find artist, album and fname
    ddb_tf_context_t context;
    context._size = sizeof(ddb_tf_context_t);
    context.it = it;
    context.iter = PL_MAIN;
    context.flags = 0;
    context.update = 0;
    context.plt = nullptr;


    // TODO adjust length maybe
#define ENTRY_LEN 256
    char fname[FILENAME_MAX];
    char artist[ENTRY_LEN];
    char album[ENTRY_LEN];
    DBAPI->pl_lock();
    strncpy(fname, DBAPI->pl_find_meta(it, ":URI"), FILENAME_MAX);
    DBAPI->pl_unlock();
    DBAPI->tf_eval (&context, script_artist_byte, artist, ENTRY_LEN);
    DBAPI->tf_eval (&context, script_album_byte, album, ENTRY_LEN);
#undef ENTRY_LEN
    return QtConcurrent::run(getArtwork,QString(fname),QString(artist),QString(album), plug);
}

char * CoverArtLegacy::getArtwork(const QString fname, const QString artist, const QString album, DB_artwork_plugin_t *plug) {
    bool r = false;
    char *o = plug->get_album_art(fname.toUtf8(), artist.toUtf8(), album.toUtf8(), -1, artwork_callback, &r);
    if (o) {
        return o;
    }
    while (!r) {
        // hope we don't get stuck in endless loop...
        QThread::msleep(100);
    }
    return plug->get_album_art(fname.toUtf8(), artist.toUtf8(), album.toUtf8(), -1, nullptr, nullptr);
}

void CoverArtLegacy::artwork_callback(const char *fname, const char *artist, const char *album, void *user_data) {
    Q_UNUSED(fname) Q_UNUSED(artist) Q_UNUSED(album)
    bool *p = static_cast<bool *>(user_data);
    *p = true;
}

const char * CoverArtLegacy::getDefaultCoverArt() {
    if (plug) {
        return plug->get_default_cover();
    }
    return nullptr;
}

void CoverArtLegacy::unloadCoverArt(const char*) {
    return;
}

// CoverArtBackend

CoverArtBackend::CoverArtBackend(QObject *parent, DB_functions_t *funcs) : QObject(parent) {
    db = funcs;
}

