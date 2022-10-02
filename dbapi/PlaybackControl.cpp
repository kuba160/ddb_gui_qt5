#include "PlaybackControl.h"

#include <QVariant>
#include <QDebug>
#include <QTimer>

#define DBAPI (this->deadbeef)


PlaybackControl::PlaybackControl(QObject *parent, DB_functions_t *api) : QObject(parent) {
    deadbeef = api;

    if (DBAPI->conf_get_int("resume.paused", 0)) {
        // will be paused
        m_state = STATE_PAUSED;
    }
    else {
        // can be playing or stopped
        m_state = DBAPI->get_output() ? (playback_state) DBAPI->get_output()->state() : STATE_STOPPED;
    }

    connect(this, &PlaybackControl::currentTrackChanged, this, &PlaybackControl::positionChanged);
    connect(this, &PlaybackControl::currentTrackChanged, this, &PlaybackControl::positionMaxChanged);

    QTimer::singleShot(2000, this, [this]() {
        m_state =  DBAPI->get_output() ?
                    (playback_state) DBAPI->get_output()->state() :
                    STATE_STOPPED;
        emit stateChanged();
    });
}



int PlaybackControl::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    // Monitor for:
    // - Volume change
    // - Position change
    // - Track length change (track change)
    // - Shuffle change
    // - Repeat change
    // - State change (track change)
    // - Current track change (track change)
    ddb_event_trackchange_t *track_change = nullptr;
    switch (id) {
        case DB_EV_VOLUMECHANGED:
            emit volumeChanged();
            break;
        case DB_EV_SEEKED:
            emit positionChanged();
            break;
        case DB_EV_PAUSED:
            m_state = p1 ? STATE_PAUSED : STATE_PLAYING;
            emit stateChanged();
            break;

        case DB_EV_SONGCHANGED:
            track_change = (ddb_event_trackchange_t *)(ctx);
            //qDebug() << "-" << track_change->from << "+" << track_change->to;
            // todo do not emit when track is looped
            if (track_change->from == nullptr) {
                m_state = STATE_PLAYING;
            }
            else if (track_change->to == nullptr) {
                m_state = STATE_STOPPED;
            }
            else {
                m_state = STATE_PLAYING;
            }
            emit stateChanged();
            /* fall through */
        case DB_EV_STOP:
            emit currentTrackChanged();
            break;
    }
    return 0;
}

void PlaybackControl::play(int index) {
    if (index <= -1) {
        DBAPI->sendmessage(DB_EV_PLAY_CURRENT, 0, 0, 0);
    }
    else {
        DBAPI->sendmessage(DB_EV_PLAY_NUM, 0, index, 0);
    }
}

void PlaybackControl::play() {
    DBAPI->sendmessage(DB_EV_PLAY_CURRENT, 0, 0, 0);
}

void PlaybackControl::pause() {
    DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
}

void PlaybackControl::next() {
    DBAPI->sendmessage(DB_EV_NEXT, 0, 0, 0);
}

void PlaybackControl::prev() {
    DBAPI->sendmessage(DB_EV_PREV, 0, 0, 0);
}

void PlaybackControl::stop() {
    DBAPI->sendmessage(DB_EV_STOP, 0, 0, 0);
}

float PlaybackControl::getVolume() {
    return DBAPI->volume_get_db();
}

void PlaybackControl::setVolume(float vol) {
    if (getVolume() != vol) { // float check?
        DBAPI->volume_set_db(vol);
        // signal will be emitted through volume event (hopefully)
    }
}

float PlaybackControl::getPosition() {
    return DBAPI->playback_get_pos();
}

float PlaybackControl::getPositionRel() {
    return DBAPI->playback_get_pos() * getPositionMax()/100.0;
}

void PlaybackControl::setPosition(float pos) {
    DBAPI->playback_set_pos(pos);
}

float PlaybackControl::getPositionMax() {
    DB_playItem_t *it = DBAPI->streamer_get_playing_track();
    float len = 0.0;
    if (it) {
        len = DBAPI->pl_get_item_duration(it);
        DBAPI->pl_item_unref(it);
    }
    return len;
}

PlaybackControl::shuffle PlaybackControl::getShuffle() {
    return (shuffle) DBAPI->streamer_get_shuffle();
}

void PlaybackControl::setShuffle(shuffle a) {
    if (getShuffle() != a) {
        DBAPI->streamer_set_shuffle((ddb_shuffle_t) a);
        emit shuffleChanged();
    }
}

PlaybackControl::repeat PlaybackControl::getRepeat() {
    return (repeat) DBAPI->streamer_get_repeat();
}

void PlaybackControl::setRepeat(repeat a) {
    if (getRepeat() != a) {
        DBAPI->streamer_set_repeat((ddb_repeat_t) a);
        emit repeatChanged();
    }
}

PlaybackControl::playback_state PlaybackControl::getState() {
    DB_output_t *out = DBAPI->get_output();
    if (out) {
        return (playback_state)out->state();
    }
    return STATE_STOPPED;
}

DB_playItem_t * PlaybackControl::getCurrentTrack() {
    return DBAPI->streamer_get_playing_track();
}

quint32 PlaybackControl::getCurrentTrackIdx() {
    DB_playItem_t *it = DBAPI->streamer_get_playing_track();
    int ret = -1;
    if (it) {
        ret = DBAPI->pl_get_idx_of(it);
        DBAPI->pl_item_unref(it);
    }
    return ret;
}

void PlaybackControl::setCurrentTrackIdx(quint32 idx) {
    DBAPI->sendmessage(DB_EV_PLAY_NUM, 0, idx, 0);
}

QString PlaybackControl::tf_current(const QString &format) {
    char* tf_format = DBAPI->tf_compile(format.toUtf8().constData());
    char buffer[1024];
    ddb_tf_context_t context;
    {
        memset(&context, 0, sizeof(context));
        context._size = sizeof(ddb_tf_context_t);
        context.it = getCurrentTrack();
        if (context.it) {
            context.plt = DBAPI->pl_get_playlist(context.it);
        }
        else {
            context.plt = nullptr;
        }
        // TODO: m_iter better handling?
        context.iter = PL_MAIN;
    }
    DBAPI->tf_eval (&context, tf_format, buffer, 1024);
    if (context.plt) {
        DBAPI->plt_unref(context.plt);
    }
    if (context.it) {
        DBAPI->pl_item_unref(context.it);
    }

    DBAPI->tf_free(tf_format);
    return QString(buffer);

}

