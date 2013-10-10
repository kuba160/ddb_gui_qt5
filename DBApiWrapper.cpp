#include "DBApiWrapper.h"

#include <QDebug>
#include "QtGui.h"

DBApiWrapper *DBApiWrapper::instance;

DBApiWrapper::DBApiWrapper() {
    isPaused = DBAPI->conf_get_int("resume.paused", 0);
}

void DBApiWrapper::Destroy() {
    DBAPI->sendmessage(DB_EV_TERMINATE, 0, 0, 0);
    delete instance;
    instance = NULL;
}

DBApiWrapper *DBApiWrapper::Instance() {
    if (instance == NULL) {
        instance = new DBApiWrapper();
    }

    return instance;
}

int DBApiWrapper::onPlaylistChanged() {
    emit WRAPPER->playlistChanged();
    return 0;
}

int DBApiWrapper::onSongChanged(ddb_event_trackchange_t *ev) {
    emit WRAPPER->trackChanged(ev->from, ev->to);
    return 0;
}

int DBApiWrapper::onPause() {
    DBApiWrapper::Instance()->isPaused = !DBApiWrapper::Instance()->isPaused;
    emit DBApiWrapper::Instance()->playbackPaused();
    return 0;
}

int DBApiWrapper::onDeadbeefActivated() {
    emit WRAPPER->deadbeefActivated();
    return 0;
}


void DBApiWrapper::addTracksByUrl(const QUrl &url, int position) {
    int pabort = 0;
    DB_playItem_t *track = (position > -1) ? DBAPI->pl_get_for_idx(position) : NULL;
    if (DBAPI->plt_insert_dir(DBAPI->plt_get_curr(), track, url.toString().toUtf8().data(), &pabort, 0, 0) == NULL) {
        DBAPI->plt_insert_file(DBAPI->plt_get_curr(), track, url.toString().toUtf8().data(), &pabort, 0, 0);
    }
    if (track)
        DBAPI->pl_item_unref(track);
}

void DBApiWrapper::sendPlayMessage(uint32_t id) {
    DBApiWrapper::Instance()->isPaused = false;
    DBAPI->sendmessage(id, 0, 0, 0);
}

void DBApiWrapper::playTrackByIndex(int index) {
    DBApiWrapper::Instance()->isPaused = false;
    DBAPI->sendmessage(DB_EV_PLAY_NUM, 0, index, 0);
}
