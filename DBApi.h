#ifndef DBAPIWRAPPER_H
#define DBAPIWRAPPER_H

#include <deadbeef/deadbeef.h>

#include <QUrl>
#include <QObject>

class DBApi : public QObject {

    Q_OBJECT

public:
    DBApi(QWidget *parent);
    ~DBApi();


    bool isPaused();


    void addTracksByUrl(const QUrl &url, int position = -1);

    int getVolume();

    ddb_playback_state_t getOutputState();

    // plugin message handler
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);
    
private:
    bool isPaused_val;

// Signals are subscribed by different parts of gui
signals:
    void volumeChanged(int);
    void playlistChanged();
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void playbackPaused();
    void deadbeefActivated();

// Slots redirect messages from qt gui to deadbeef internal system
public slots:
    // When user changed volume:
    void setVolume(int);
    //
    void playTrackByIndex(int);
    //
    void sendPlayMessage(uint32_t id);
    //
    void togglePause();
};

#endif // DBAPIWRAPPER_H
