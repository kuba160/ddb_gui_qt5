#ifndef COVERARTBACKEND_H
#define COVERARTBACKEND_H

#include <QObject>
#include <QFuture>

#include <deadbeef/deadbeef.h>
#include "include/artwork.h"
#undef __ARTWORK_H
#include "include/artwork2.h"

#include "CoverArt.h"

class CoverArtLegacy : public ICoverArtBackend {
public:
    CoverArtLegacy(QObject *parent, DB_functions_t *api);
    ~CoverArtLegacy();
    // virtual functions implementation
    QFuture<char *> loadCoverArt(DB_playItem_t *);
    const char *getDefaultCoverArt();
    void unloadCoverArt(const char*);
protected:
    // static functions for threaded callback
    static char *getArtwork(const QString fname, const QString artist, const QString album, DB_artwork_plugin_t *plug);
    static void artwork_callback(const char *fname, const char *artist, const char *album, void *user_data);
    DB_functions_t *db;
    DB_artwork_plugin_t *plug;
    char *script_album_byte = nullptr;
    char *script_artist_byte = nullptr;
};

class CoverArtNew : public ICoverArtBackend {
    DB_functions_t *db;
    QHash<const char *, ddb_cover_info_t *> ht;
    ddb_artwork_plugin_t *plug;
public:
    CoverArtNew(QObject *parent, DB_functions_t *api);
    // virtual functions implementation
    QFuture<char *> loadCoverArt(DB_playItem_t *);
    const char *getDefaultCoverArt();
    void unloadCoverArt(const char *);
protected:
    // static functions for threaded callback
    static char *getArtwork(DB_playItem_t *it, CoverArtNew *can);

    static void artwork_callback(int error, ddb_cover_query_t *query, ddb_cover_info_t *cover);
};

#endif // COVERARTBACKEND_H
