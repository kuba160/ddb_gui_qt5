#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include <QObject>
#include <deadbeef/deadbeef.h>

class PlaybackControl : public QObject
{
    Q_OBJECT
    friend class DBApi;
    DB_functions_t *deadbeef;
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);
public:
    explicit PlaybackControl(QObject *parent, DB_functions_t *api);

    enum playback_state
    {
        STATE_STOPPED = DDB_PLAYBACK_STATE_STOPPED,
        STATE_PLAYING = DDB_PLAYBACK_STATE_PLAYING,
        STATE_PAUSED = DDB_PLAYBACK_STATE_PAUSED,
    };
    Q_ENUM(playback_state)

    enum shuffle {
        SHUFFLE_OFF = DDB_SHUFFLE_OFF,
        SHUFFLE_TRACKS = DDB_SHUFFLE_TRACKS,
        SHUFFLE_RANDOM = DDB_SHUFFLE_RANDOM,
        SHUFFLE_ALBUMS = DDB_SHUFFLE_ALBUMS,
    };
    Q_ENUM(shuffle)

    enum repeat {
        REPEAT_ALL = DDB_REPEAT_ALL,
        REPEAT_OFF = DDB_REPEAT_OFF,
        REPEAT_SINGLE = DDB_REPEAT_SINGLE,
    };
    Q_ENUM(repeat)

    Q_PROPERTY(float volume READ getVolume WRITE setVolume NOTIFY volumeChanged) // volume in dB
    Q_PROPERTY(float position_abs READ getPosition WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(float position_rel READ getPositionRel NOTIFY positionChanged)
    Q_PROPERTY(float position_max READ getPositionMax NOTIFY positionMaxChanged) // track length
    Q_PROPERTY(shuffle shuffle READ getShuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(repeat repeat READ getRepeat WRITE setRepeat NOTIFY repeatChanged)
    Q_PROPERTY(playback_state state MEMBER m_state NOTIFY stateChanged)
    //Q_PROPERTY(DB_playItem_t * currentTrack READ getCurrentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(quint32 currentTrackIdx READ getCurrentTrackIdx WRITE setCurrentTrackIdx NOTIFY currentTrackChanged) // NOTE: won't be changed when track is moved

    Q_PROPERTY(bool playing READ getPlaying NOTIFY stateChanged)
    Q_PROPERTY(bool paused READ getPaused NOTIFY stateChanged)
    Q_PROPERTY(bool stopped READ getStopped NOTIFY stateChanged)

public slots:
    void play();
    void play(int index);
    void pause(); // toggle pause
    void next();
    void prev();
    void stop();

signals:
    void volumeChanged();
    void positionChanged();
    void positionMaxChanged();
    void shuffleChanged();
    void repeatChanged();
    void stateChanged();
    void currentTrackChanged();

public:
    float getVolume();
    void setVolume(float);
    float getPosition();
    void setPosition(float);
    float getPositionMax();
    float getPositionRel();
    shuffle getShuffle();
    void setShuffle(shuffle);
    repeat getRepeat();
    void setRepeat(repeat);
    playback_state getState();
    // state read-only
    DB_playItem_t *getCurrentTrack(); // remember to unref
    quint32 getCurrentTrackIdx();
    void setCurrentTrackIdx(quint32);

    inline bool getPlaying() {
        return m_state == STATE_PLAYING;
    }

    inline bool getPaused() {
        return m_state == STATE_PAUSED;
    }

    inline bool getStopped() {
        return m_state == STATE_STOPPED;
    }

public slots:
    QString tf_current(const QString &format);

private:
    playback_state m_state;

};

#endif // PLAYBACKCONTROL_H
