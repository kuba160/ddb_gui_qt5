// This file defines class that allows interraction with deadbeef
// TODO

#ifndef PLAYITEMITERATOR_H
#define PLAYITEMITERATOR_H

#include <deadbeef/deadbeef.h>
#include <QObject>
#include <QMimeData>

class PlayItemIterator: public QObject {
    Q_OBJECT
public:
    enum ContextType {
        NONE,
        CURRENT_TRACK,
        TRACKS,
        PLAYLIST,
        PLAYLIST_SELECTED // TODO IGNORE FOR NOW
    };

    // duplicate
    PlayItemIterator(const PlayItemIterator &from);
    // create playlist action context
    PlayItemIterator(ddb_playlist_t* plt);
    // create playitem list action context
    PlayItemIterator(QList<DB_playItem_t*> tracks);
    PlayItemIterator(DB_playItem_t* track);
    // create main or nowplaying action context
    PlayItemIterator(bool nowplaying = false);
    //PlayItemIterator() : PlayItemIterator(false) {}; // none for no arguments
    // url
    PlayItemIterator(QUrl url);

    DB_playItem_t * getNextIter();
    void resetIter();

    ~PlayItemIterator();

    void apply_legacy(DB_plugin_action_t *action);

    QMimeData* toMimeData();

    static PlayItemIterator fromMimeData(QMimeData &mime);


    ContextType contextType() {return d_type;};

protected:
    ContextType d_type;
    ddb_playlist_t* d_playlist; // for playlist/playlist_selected
    QList<DB_playItem_t *> d_tracklist; // for tracks
    // iter pos
    uint16_t d_iter_pos = 0;
};

typedef QList<DB_playItem_t *> playItemList;

QDataStream &operator<<(QDataStream &ds, const playItemList &pil);
QDataStream &operator>>(QDataStream &ds, playItemList &pil);

class PlayItemMimeData : public QMimeData {
    Q_OBJECT
    DB_functions_t *api;
public:
    PlayItemMimeData(DB_functions_t *ddb, PlayItemIterator pit);
    PlayItemMimeData(DB_functions_t *ddb, QList<DB_playItem_t*>);
    ~PlayItemMimeData();
    static QList<DB_playItem_t*> getTracks(const QMimeData *mime);

};

#endif // PLAYITEMITERATOR_H
