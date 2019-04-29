#ifndef DBAPIWRAPPER_H
#define DBAPIWRAPPER_H

#include <deadbeef/deadbeef.h>

#include <QUrl>
#include <QObject>

#define WRAPPER DBApiWrapper::Instance()

class DBApiWrapper : public QObject {

    Q_OBJECT

public:
    static DBApiWrapper *Instance();
    static void Destroy();

    bool isPaused;

    void sendPlayMessage(uint32_t id);

    void playTrackByIndex(int index);

    void addTracksByUrl(const QUrl &url, int position = -1);
    
    static int onSongChanged(ddb_event_trackchange_t *ev);
    static int onPause(int paused);
    static int onPlaylistChanged();
    static int onDeadbeefActivated();
    
private:
    DBApiWrapper();
    static DBApiWrapper *instance;

signals:
    void playlistChanged();
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void playbackPaused();
    void deadbeefActivated();
};

#endif // DBAPIWRAPPER_H
