#include "DBApi.h"

#include <QDebug>
#include <QWidget>
#include "QtGui.h"

#undef DBAPI
#define DBAPI this->deadbeef


DBApi::DBApi(QWidget *parent, DB_functions_t *Api) : QObject(parent) {
    this->deadbeef = Api;
    if (DBAPI->conf_get_int("resume.paused", 0)) {
        // will be paused
        internal_state = DDB_PLAYBACK_STATE_PAUSED;
    }
    else {
        // can be playing or stopped
        internal_state = DDB_PLAYBACK_STATE_PLAYING;
    }
}

DBApi::~DBApi() {
    plugin.plugin.message = nullptr;
}

int DBApi::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    Q_UNUSED(p2);

    ddb_event_trackchange_t *ev;
    switch (id) {
        case DB_EV_SONGCHANGED:
            ev = (ddb_event_trackchange_t *)ctx;
            emit trackChanged(ev->from, ev->to);
            break;
        case DB_EV_PAUSED:
            //internal_state = p1;
            if (internal_state != p1) {
                internal_state = (ddb_playback_state_t) p1;
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
            break;
        case DB_EV_TRACKINFOCHANGED:
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


bool DBApi::isPaused() {
    return (internal_state == DDB_PLAYBACK_STATE_PAUSED) ? true : false;
}

int DBApi::getVolume() {
    return static_cast<int>(DBAPI->volume_get_db());
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

void DBApi::setVolume(int value) {
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

DBToolbarWidget::DBToolbarWidget(QWidget *parent, DBApi *api_a) {
    Q_UNUSED(parent);
    Q_UNUSED(api);
    api = api_a;
}

DBToolbarWidget::~DBToolbarWidget() {
    // exit
}

void DBToolbarWidget::loadConfig(QObject *ptr) {
    Q_UNUSED(ptr);
    ;
}
