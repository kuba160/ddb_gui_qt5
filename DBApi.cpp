#include "DBApi.h"

#include <QDebug>
#include <QWidget>
#include "QtGui.h"
#include "CoverArtCache.h"
#include "QtGuiSettings.h"
#include "ActionManager.h"
#include "DeadbeefTranslator.h"
#undef _
#undef DBAPI
#define DBAPI this->deadbeef

#define CAC (COVERARTCACHE_P(coverart_cache))
#define CSET (static_cast<QtGuiSettings *>(qt_settings))
#define AM (static_cast<ActionManager *>(action_manager))


DBApi::DBApi(QObject *parent, DB_functions_t *Api) : QObject(parent), coverart_cache(parent) {
    this->deadbeef = Api;
    if (DBAPI->conf_get_int("resume.paused", 0)) {
        // will be paused
        internal_state = DDB_PLAYBACK_STATE_PAUSED;
    }
    else {
        // can be playing or stopped
        internal_state = DDB_PLAYBACK_STATE_PLAYING;
    }


    // playlists
    {
        int playlistCount = DBAPI->plt_get_count();
        char title[100];
        for (int i = 0; i < playlistCount; i++) {
            DBAPI->pl_lock();
            DBAPI->plt_get_title(DBAPI->plt_get_for_idx(i), title, sizeof(title));
            DBAPI->pl_unlock();
            playlistNames.push_back(QString::fromUtf8(title));
            strcpy(title, "");
        }
    }

    // shuffle/repeat
    currShuffle = DBAPI->streamer_get_shuffle();
    currRepeat = DBAPI->streamer_get_repeat();

    // CoverArt Cache
    coverart_cache = new CoverArtCache();
    if(COVERARTCACHE_P(coverart_cache)->getCoverArtPlugin()) {
        connect(this, SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)),COVERARTCACHE_P(coverart_cache), SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    }
    connect(&COVERARTCACHE_P(coverart_cache)->currCover,SIGNAL(finished()),this,SLOT(onCurrCoverChanged()));

    // Settings
    qt_settings = new QtGuiSettings(this);

    // Action Manager
    action_manager = new ActionManager(this,this);
}

DBApi::~DBApi() {
    plugin.plugin.message = nullptr;
    delete CAC;
    delete CSET;
}

const char * DBApi::_(const char *str) {
    return dbtr->translate(nullptr, str).toUtf8();
}

int DBApi::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    Q_UNUSED(p2);

    ddb_event_trackchange_t *ev;
     ddb_playback_state_t state;
    switch (id) {
        case DB_EV_SONGCHANGED:
            ev = (ddb_event_trackchange_t *)ctx;
            emit trackChanged(ev->from, ev->to);
            emit queueChanged();
            break;
        case DB_EV_PAUSED:
            state = p1 ? DDB_PLAYBACK_STATE_PAUSED : DDB_PLAYBACK_STATE_PLAYING;
            if (internal_state != state) {
                internal_state = state;
                if (p1) {
                    emit playbackPaused();
                }
                else {
                    emit playbackUnPaused();
                    emit playbackStarted();
                }
            }
            break;
        case DB_EV_PLAYLISTCHANGED:
            emit playlistChanged();
            break;
        case DB_EV_ACTIVATED:
            emit deadbeefActivated();
            break;
        case DB_EV_VOLUMECHANGED:
            emit volumeChanged(getVolume());
            break;
        case DB_EV_PLAY_NUM:
        case DB_EV_PLAY_CURRENT:
        case DB_EV_PLAY_RANDOM:
        case DB_EV_SONGSTARTED:
            emit playbackStarted();
            break;
        case DB_EV_STOP:
            emit playbackStopped();
            emit queueChanged();
            break;
        case DB_EV_TRACKINFOCHANGED:
            // detect queue
            if (p1 == DDB_PLAYLIST_CHANGE_PLAYQUEUE) {
                emit queueChanged();
                ddb_event_track_t *track_event = reinterpret_cast<ddb_event_track_t *>(ctx);
                if (DBAPI->playqueue_get_count() > queue_count) {
                    qDebug() <<"queue: track added";
                    emit queueTrackAdded(track_event->track);
                }
                else {
                    qDebug() <<"queue: track removed";
                    emit queueTrackRemoved(track_event->track);
                }
            }

            ddb_playback_state_t output_state = DBAPI->get_output()->state();
            if (internal_state != output_state) {
                internal_state = output_state;
                if (internal_state == DDB_PLAYBACK_STATE_PAUSED) {
                    emit playbackPaused();
                }
                else if (internal_state == DDB_PLAYBACK_STATE_STOPPED) {
                    emit playbackStopped();
                }
                else {
                    // DDB_PLAYBACK_STATE_PLAYING
                    emit playbackUnPaused();
                    emit playbackStarted();
                }
            }
            break;
    }
    return 0;
}

QString const& DBApi::playlistNameByIdx(int idx) {
    static QString empty;
    if (idx >= playlistNames.size()) {
        return empty;
    }
    return playlistNames.at(idx);
}

unsigned long DBApi::getPlaylistCount() {
    return playlistNames.size();
}

bool DBApi::isPaused() {
    return (internal_state == DDB_PLAYBACK_STATE_PAUSED) ? true : false;
}

float DBApi::getVolume() {
    return DBAPI->volume_get_db();
}

ddb_playback_state_t DBApi::getOutputState() {
    if (DBAPI->get_output()) {
        return DBAPI->get_output()->state();
    }
    return DDB_PLAYBACK_STATE_STOPPED;
}

ddb_playback_state_t DBApi::getInternalState() {
    return internal_state;
}

void DBApi::playItemContextMenu(QPoint p, DB_playItem_t *it) {
    AM->playItemContextMenu(p,it);
}

void DBApi::confSetValue(const QString &plugname, const QString &key, const QVariant &value) {
    settings->setValue(plugname,key,value);
}

QVariant DBApi::confGetValue(const QString &plugname, const QString &key, const QVariant &defaultValue) {
    return settings->getValue(plugname,key,defaultValue);
}

void DBApi::autoSetValue(void *widget, const QString &key, const QVariant &value) {
    settings->autoSetValue(widget,key,value);
}

QVariant DBApi::autoGetValue(void *widget, const QString &key, const QVariant &defaultValue) {
    return settings->autoGetValue(widget, key, defaultValue);
}

void DBApi::addTracksByUrl(const QUrl &url, int position) {
    int pabort = 0;
    DB_playItem_t *track = (position > -1) ? DBAPI->pl_get_for_idx(position) : nullptr;
    if (DBAPI->plt_insert_dir(DBAPI->plt_get_curr(), track, url.toString().toUtf8().data(), &pabort, nullptr, nullptr) == nullptr) {
        DBAPI->plt_insert_file(DBAPI->plt_get_curr(), track, url.toString().toUtf8().data(), &pabort, nullptr, nullptr);
    }
    if (track)
        DBAPI->pl_item_unref(track);
}


// slots

void DBApi::setVolume(float value) {
    DBAPI->volume_set_db(value);
}

void DBApi::playTrackByIndex(uint32_t index) {
    DBAPI->sendmessage(DB_EV_PLAY_NUM, 0, index, 0);
}

void DBApi::sendPlayMessage(uint32_t id) {
    DBAPI->sendmessage(id, 0, 0, 0);
}

void DBApi::togglePause() {
    DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
}

void DBApi::play() {
    DBAPI->sendmessage(DB_EV_PLAY_CURRENT, 0, 0, 0);
}

void DBApi::stop() {
    DBAPI->sendmessage(DB_EV_STOP, 0, 0, 0);
}

void DBApi::playNext() {
    DBAPI->sendmessage(DB_EV_NEXT, 0, 0, 0);
}

void DBApi::playPrev() {
    DBAPI->sendmessage(DB_EV_PREV, 0, 0, 0);
}

void DBApi::changePlaylist(int idx) {
    if (idx < playlistNames.size() && playlist_internal != idx) {
        playlist_internal = idx;
        DBAPI->plt_set_curr_idx(idx);
        DBAPI->conf_set_int("playlist.current", idx);
        emit playlistChanged(idx);
        emit playlistChanged();
    }
}

void DBApi::movePlaylist(int plt, int before) {
    if (plt != before) {
       DBAPI->plt_move(plt, before);
       playlistNames.move(plt, before);
       if (plt == playlist_internal) {
           playlist_internal = before;
       }
       emit playlistMoved(plt, before);
    }
}

void DBApi::newPlaylist(QString *name) {
    DBAPI->plt_add (-1, name->toUtf8());
    playlistNames.append(*name);
    emit playlistCreated();
}

void DBApi::renamePlaylist(int plt, const QString *name) {
    if (plt < playlistNames.size()) {
        DBAPI->pl_lock ();
        ddb_playlist_t *plt_p = DBAPI->plt_get_for_idx(plt);
        DBAPI->plt_set_title (plt_p, name->toUtf8());
        DBAPI->plt_unref (plt_p);
        DBAPI->pl_unlock ();
        playlistNames.insert(plt, *name);
        playlistNames.removeAt(plt+1);
        emit playlistRenamed(plt);
    }
}

void DBApi::setShuffle(ddb_shuffle_t i) {
    DBAPI->streamer_set_shuffle(i);
    currShuffle = i;
    emit shuffleChanged();
}

void DBApi::setRepeat(ddb_repeat_t i) {
    DBAPI->streamer_set_repeat(i);
    currRepeat = i;
    emit repeatChanged();
}

bool DBApi::isCoverArtPluginAvailable() {
    return CAC->getCoverArtPlugin() ? true : false;
}

QFuture<QImage *> DBApi::loadCoverArt(const char *fname, const char *artist, const char *album) {
    return CAC->loadCoverArt(fname,artist,album);
}

QFuture<QImage *> DBApi::loadCoverArt(DB_playItem_t *p) {
    return CAC->loadCoverArt(p);
}

QImage * DBApi::getDefaultCoverArt() {
    return CAC->getDefaultCoverArt();
}

void DBApi::coverArt_unref(QImage *) {
    // TODO
    return;
}

void DBApi::onCurrCoverChanged() {
    emit currCoverChanged(CAC->currCover.result());
}

DBWidget::DBWidget(QWidget *parent, DBApi *api_a) {
    if (api_a == nullptr) {
        qDebug() << "Widget (" << parent <<") initialized without api pointer!";
    }
    api = api_a;
    if (parent) {
        _internalNameWidget = parent->objectName();
        //qDebug() << _internalNameWidget << Qt::endl;
    }
}

DBWidget::~DBWidget() {
    // exit
    //delete _internalNameWidget;
}

QDataStream &operator<<(QDataStream &ds, const playItemList &pil) {
    ds << pil.count;
    qint32 i;
    for (i = 0; i < pil.count; i++) {
        auto ptr= reinterpret_cast<quintptr>(pil.list.at(i));
        ds << ptr;
    }
    return ds;
}
QDataStream& operator >> (QDataStream &ds, playItemList &pil) {
    pil.list.clear();
    ds >> pil.count;
    qint32 i;
    for (i = 0; i < pil.count; i++) {
        quintptr ptrval;
        ds >> ptrval;
        auto temp = reinterpret_cast<DB_playItem_t *>(ptrval);
        pil.list.append(temp);
    }
    return ds;
}
