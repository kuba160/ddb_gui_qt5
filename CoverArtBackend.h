#ifndef COVERARTBACKEND_H
#define COVERARTBACKEND_H

#include <QObject>
#include <QFuture>

#include <DBApi.h>
#include "include/artwork.h"
#undef __ARTWORK_H
#include "include/artwork2.h"

// abstract CoverArtBackend definition
class CoverArtBackend : public QObject {
    Q_OBJECT
public:
    explicit CoverArtBackend(QObject *parent = nullptr, DB_functions_t *funcs = nullptr);

    virtual QFuture<char *> loadCoverArt(DB_playItem_t *) = 0;
    virtual const char * getDefaultCoverArt() = 0;
    virtual void unloadCoverArt(const char*) = 0;

    DB_functions_t *db;
};

class CoverArtLegacy : public CoverArtBackend {
    Q_OBJECT
public:
    CoverArtLegacy(QObject *parent = nullptr, DB_functions_t *funcs = nullptr);
    ~CoverArtLegacy();
    // virtual functions implementation
    QFuture<char *> loadCoverArt(DB_playItem_t *);
    const char *getDefaultCoverArt();
    void unloadCoverArt(const char*);
    // static functions for threaded callback
    static char *getArtwork(const QString fname, const QString artist, const QString album, DB_artwork_plugin_t *plug);
    static void artwork_callback(const char *fname, const char *artist, const char *album, void *user_data);
protected:
    DB_artwork_plugin_t *plug;
    char *script_album_byte = nullptr;
    char *script_artist_byte = nullptr;
};

class CoverArtNew : public CoverArtBackend {
    Q_OBJECT
public:
    CoverArtNew(QObject *parent = nullptr, DB_functions_t *funcs = nullptr);
    // virtual functions implementation
    QFuture<char *> loadCoverArt(DB_playItem_t *);
    const char *getDefaultCoverArt();
    void unloadCoverArt(const char *);
    // static functions for threaded callback
    static char *getArtwork(DB_playItem_t *it, CoverArtNew *can);
    static void artwork_callback(int error, ddb_cover_query_t *query, ddb_cover_info_t *cover);
    // variables
    QHash<const char *, ddb_cover_info_t *> ht;
    ddb_artwork_plugin_t *plug;
};

#endif // COVERARTBACKEND_H
