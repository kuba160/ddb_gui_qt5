#include "DBApi.h"

#include <QDebug>
#include <QWidget>
#include "QtGui.h"

// defined in QtGui.cpp
extern DB_functions_t *deadbeef;
extern DB_gui_t plugin;

DBApi::DBApi(QWidget *parent) : QObject(parent) {
    isPaused_val = DBAPI->conf_get_int("resume.paused", 0);
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
            isPaused_val = p1;
            emit playbackPaused();
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

    }
    return 0;
}


bool DBApi::isPaused() {
    return isPaused_val;
}

int DBApi::getVolume() {
    return DBAPI->volume_get_db();
}

ddb_playback_state_t DBApi::getOutputState() {
    if (DBAPI->get_output()) {
        return DBAPI->get_output()->state();
    }
    return DDB_PLAYBACK_STATE_STOPPED;
}

void DBApi::addTracksByUrl(const QUrl &url, int position) {
    int pabort = 0;
    DB_playItem_t *track = (position > -1) ? DBAPI->pl_get_for_idx(position) : NULL;
    if (DBAPI->plt_insert_dir(DBAPI->plt_get_curr(), track, url.toString().toUtf8().data(), &pabort, 0, 0) == NULL) {
        DBAPI->plt_insert_file(DBAPI->plt_get_curr(), track, url.toString().toUtf8().data(), &pabort, 0, 0);
    }
    if (track)
        DBAPI->pl_item_unref(track);
}


// slots

void DBApi::setVolume(int value) {
    DBAPI->volume_set_db(value);
}

void DBApi::playTrackByIndex(int index) {
    isPaused_val = false;
    DBAPI->sendmessage(DB_EV_PLAY_NUM, 0, index, 0);
}

void DBApi::sendPlayMessage(uint32_t id) {
    isPaused_val = false;
    DBAPI->sendmessage(id, 0, 0, 0);
}

void DBApi::togglePause() {
    DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
}
