#include "PlayItemIterator.h"

#include <QDebug>
#include <QDataStream>
#include <QMimeData>
#include <QIODevice>
#include <QUrl>

extern DB_functions_t *DBApi_deadbeef;
int plt_refc = 0;
int it_refc = 0;

#define DBAPI (DBApi_deadbeef)

PlayItemIterator::PlayItemIterator(const PlayItemIterator &from) {
    this->d_type = from.d_type;
    this->d_playlist = from.d_playlist;
    this->d_tracklist = from.d_tracklist;

    if (d_type == PlayItemIterator::PLAYLIST || d_type == PlayItemIterator::PLAYLIST_SELECTED) {
        DBAPI->plt_ref(d_playlist);
        plt_refc++;
    }
    else if (d_type == PlayItemIterator::TRACKS) {
        for (DB_playItem_t *it : d_tracklist) {
            DBAPI->pl_item_ref(it);
            it_refc++;
        }
    }
    // NONE or CURRENT_TRACK, nothing to copy
}

PlayItemIterator::PlayItemIterator(ddb_playlist_t* plt) {
    this->d_playlist = plt;
    DBAPI->plt_ref(d_playlist);
    plt_refc++;
    this->d_type = PlayItemIterator::PLAYLIST;
}

PlayItemIterator::PlayItemIterator(QList<DB_playItem_t*> tracks) {
    this->d_type = PlayItemIterator::TRACKS;
    this->d_tracklist = tracks;
    for (DB_playItem_t *it : d_tracklist) {
        DBAPI->pl_item_ref(it);
        it_refc++;
    }
}

PlayItemIterator::PlayItemIterator(DB_playItem_t* track) {
    this->d_type = PlayItemIterator::TRACKS;
    this->d_tracklist = QList<DB_playItem_t*>();
    this->d_tracklist.append(track);
    DBAPI->pl_item_ref(track);
    it_refc++;
}

PlayItemIterator::PlayItemIterator(bool nowplaying) {
    if (nowplaying) {
        this->d_type = PlayItemIterator::CURRENT_TRACK;
    }
    else {
        this->d_type = PlayItemIterator::NONE;
    }
}

PlayItemIterator::PlayItemIterator(QUrl url) {
    this->d_type = PlayItemIterator::NONE;
    DB_playItem_t *track = DBAPI->pl_item_init(url.toEncoded().constData());
    if (track) {
        this->d_type = PlayItemIterator::TRACKS;
        this->d_tracklist = QList<DB_playItem_t*>();
        this->d_tracklist.append(track);
    }
}

PlayItemIterator::~PlayItemIterator() {
    if (d_type == PlayItemIterator::PLAYLIST) {
        for (DB_playItem_t *it : d_tracklist) {
            DBAPI->pl_item_unref(it);
            it_refc--;
        }
        plt_refc--;
        DBAPI->plt_unref(d_playlist);
    }
    else if (d_type == PlayItemIterator::TRACKS) {
        for (DB_playItem_t *it : d_tracklist) {
            DBAPI->pl_item_unref(it);
            it_refc--;
        }
    }
    if (this->d_type != PlayItemIterator::NONE) {
        qDebug() << "REFC PLT:" << plt_refc << "IT:" << it_refc;
    }
}

DB_playItem_t * PlayItemIterator::getNextIter() {
    DB_playItem_t *it = nullptr;
    switch(d_type) {
        case PlayItemIterator::NONE:
            break;
        case PlayItemIterator::CURRENT_TRACK:
            if (d_iter_pos == 0) {
                // we change to tracks
                it = DBAPI->streamer_get_playing_track();
                // TODO UNREF!!!
                //if (it)
                it_refc++;
                d_type = PlayItemIterator::TRACKS;
            }
            break;
        case PlayItemIterator::TRACKS:
            if (d_iter_pos < d_tracklist.count())
                it = d_tracklist.at(d_iter_pos);
            break;
        case PlayItemIterator::PLAYLIST:
            it = DBAPI->plt_get_item_for_idx(d_playlist, d_iter_pos, PL_MAIN);
            if (it) {
                d_tracklist.append(it);
                it_refc++;
            }
            break;
        case PlayItemIterator::PLAYLIST_SELECTED:
            int item_count = DBAPI->plt_get_item_count(d_playlist, PL_MAIN);
            while (it == nullptr && d_iter_pos < item_count) {
                DB_playItem_t *it_next = DBAPI->plt_get_item_for_idx(d_playlist, d_iter_pos, PL_MAIN);
                if (DBAPI->pl_is_selected(it_next)) {
                    it = it_next;
                    d_tracklist.append(it);
                    it_refc++;
                }
                else {
                    DBAPI->pl_item_unref(it_next);
                    d_iter_pos++;
                }
            }
            break;
    }
    if (it) {
        d_iter_pos++;
    }
    return it;
}

void PlayItemIterator::resetIter() {
    d_iter_pos = 0;
    if (d_type == ContextType::PLAYLIST) {
        for (DB_playItem_t *it : d_tracklist) {
            DBApi_deadbeef->pl_item_unref(it);
            it_refc--;
        }
        d_tracklist.clear();
    }
}

void PlayItemIterator::apply_legacy(DB_plugin_action_t *action) {
    ddb_action_context_t context = DDB_ACTION_CTX_COUNT;
    switch(d_type) {
        case PlayItemIterator::NONE:
            context = DDB_ACTION_CTX_MAIN;
            break;
        case PlayItemIterator::CURRENT_TRACK:
            context = DDB_ACTION_CTX_NOWPLAYING;
            break;
        case PlayItemIterator::PLAYLIST:
            context = DDB_ACTION_CTX_PLAYLIST;
            break;
        case PlayItemIterator::PLAYLIST_SELECTED:
            context = DDB_ACTION_CTX_SELECTION;
            break;
        case PlayItemIterator::TRACKS:
            // DDB_ACTION_CTX_SELECTION
            break;
    }
    if (context == DDB_ACTION_CTX_PLAYLIST ||
        context == DDB_ACTION_CTX_SELECTION) {
        DBAPI->action_set_playlist(d_playlist);
    }
    if (context == DDB_ACTION_CTX_MAIN ||
        context == DDB_ACTION_CTX_NOWPLAYING ||
        context == DDB_ACTION_CTX_PLAYLIST ||
        context == DDB_ACTION_CTX_SELECTION) {
            action->callback2(action, context);
            return;
    }
    // iterator is a track list, has to be converted to playlist
    context = DDB_ACTION_CTX_PLAYLIST;
    d_playlist = DBAPI->plt_alloc("_Q_PLAYITEMITERATOR");
    plt_refc++;

    //
    DB_playItem_t *it_prev = nullptr;
    for (DB_playItem_t *it_source : d_tracklist) {
        DB_playItem_t *it = DBAPI->pl_item_alloc();
        it_refc++;
        DBAPI->pl_item_copy(it, it_source);
        it_prev = DBAPI->plt_insert_item(d_playlist, it_prev, it);
        DBAPI->plt_item_set_selected(d_playlist, it_prev, true);
    }
    DBAPI->action_set_playlist(d_playlist);
    action->callback2(action, context);
    // clear tracks??
    DBAPI->plt_unref(d_playlist);
    plt_refc--;
    d_playlist = nullptr;
    return;
}

QMimeData* PlayItemIterator::toMimeData() {
    QList<DB_playItem_t *> list;
    DB_playItem_t *it = nullptr;
    resetIter();
    while ((it = this->getNextIter())) {
        DB_playItem_t *it_copy = DBApi_deadbeef->pl_item_alloc();
        DBApi_deadbeef->pl_item_copy(it_copy, it);
        list.append(it_copy);
    }

    QMimeData *out = new PlayItemMimeData(DBApi_deadbeef, list);
    resetIter();
    for (DB_playItem_t *it : list) {
        DBApi_deadbeef->pl_item_unref(it);
    }
    return out;
}

PlayItemIterator PlayItemIterator::fromMimeData(QMimeData &mime) {
    if (mime.hasFormat("ddb_gui_q/playitemdata")) {
        QList<DB_playItem_t *> it_list;
        QByteArray arr = mime.data("ddb_gui_q/playitemdata");
        QDataStream ds(arr);
        if (!arr.isEmpty()) {
            while(!ds.atEnd()) {
                quintptr tmp;
                ds >> tmp;
                it_list.append(reinterpret_cast<DB_playItem_t *>(tmp));
            }
        }
        return PlayItemIterator(it_list);
    }
    return PlayItemIterator();
}

QDataStream &operator<<(QDataStream &ds, const playItemList &pil) {
    qint32 i;
    for (i = 0; i < pil.length(); i++) {
        auto ptr= reinterpret_cast<quintptr>(pil[i]);
        ds << ptr;
    }
    return ds;
}
QDataStream& operator >> (QDataStream &ds, playItemList &pil) {
    pil.clear();
    while (!ds.atEnd()) {
        quintptr ptrval;
        ds >> ptrval;
        auto temp = reinterpret_cast<DB_playItem_t *>(ptrval);
        pil.append(temp);
    }
    return ds;
}

//PlayItemMimeData::PlayItemMimeData(DB_functions_t *ddb, PlayItemIterator pit) {
//    api = ddb;
//    QByteArray arr;
//    QDataStream ds(&arr, QIODevice::WriteOnly);
//    DB_playItem_t *it;
//    while((it = pit.getNextIter())) {
//        api->pl_item_ref(it);
//        ds << reinterpret_cast<quintptr>(it);
//    }
//    setData("ddb_gui_q/playitemdata", arr);
//}

PlayItemMimeData::PlayItemMimeData(DB_functions_t *ddb, QList<DB_playItem_t*> list) {
    api = ddb;
    QByteArray arr;
    QDataStream ds(&arr, QIODevice::WriteOnly);
    for (DB_playItem_t *it : list) {
        api->pl_item_ref(it);
        ds << reinterpret_cast<quintptr>(it);
    }
    setData("ddb_gui_q/playitemdata", arr);
}

QList<DB_playItem_t*> PlayItemMimeData::getTracks(const QMimeData *mime) {
    QList<DB_playItem_t*> ret;
    if (mime->hasFormat("ddb_gui_q/playitemdata")) {
        QByteArray arr = mime->data("ddb_gui_q/playitemdata");
        QDataStream ds(arr);
        if (!arr.isEmpty()) {
            while(!ds.atEnd()) {
                quintptr tmp;
                ds >> tmp;
                ret.append(reinterpret_cast<DB_playItem_t *>(tmp));
            }
        }
    }
    return ret;
}

PlayItemMimeData::~PlayItemMimeData() {
    QList<DB_playItem_t*> tracks = getTracks(this);
    for (DB_playItem_t *it : qAsConst(tracks)) {
        api->pl_item_unref(it);
    }
}
