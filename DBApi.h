// This file defines class that allows interraction with deadbeef
// TODO

#ifndef DBAPIWRAPPER_H
#define DBAPIWRAPPER_H

#include <deadbeef/deadbeef.h>

#include <QUrl>
#include <QObject>
#include <QToolBar>
#include <QDockWidget>
#include <QtGuiSettings.h>
#include "DeadbeefTranslator.h"

//#define DBAPI (this->api->deadbeef)
#define DBAPI deadbeef_internal

#define DBAPI_VMAJOR 0
#define DBAPI_VMINOR 1

class DBApi : public QObject {
    Q_OBJECT
public:
    DBApi(QWidget *parent = nullptr, DB_functions_t *Api = nullptr);
    ~DBApi();

    DB_functions_t *deadbeef = nullptr;
    const char DBApi_vmajor = DBAPI_VMAJOR;
    const char DBApi_vminor = DBAPI_VMINOR;


    bool isPaused();


    void addTracksByUrl(const QUrl &url, int position = -1);

    float getVolume();

    ddb_playback_state_t getOutputState();
    ddb_playback_state_t getInternalState();

    // plugin message handler
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);


    QString const& playlistNameByIdx(int idx);
    unsigned long getPlaylistCount();

    //
    void autoSetValue(void *widget, const QString &key, const QVariant &value);
    QVariant autoGetValue(void *widget, const QString &key, const QVariant &defaultValue);
    
private:
    ddb_playback_state_t internal_state;
    QtGuiSettings *qt_settings;

    QStringList playlistNames;

    int playlist_internal = -1;

    ddb_repeat_t currRepeat;
    ddb_shuffle_t currShuffle;

// Signals are subscribed by different parts of gui
signals:
    void volumeChanged(float);
    void playlistChanged();
    void playlistChanged(int);
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void playbackPaused();
    void playbackUnPaused();
    void playbackStarted();
    void playbackStopped();
    void deadbeefActivated();
    void playlistMoved(int plt, int before);
    void playlistCreated();
    void playlistRenamed(int plt);
    void shuffleChanged();
    void repeatChanged();

// Slots redirect messages from qt gui to deadbeef internal system
public slots:
    // When user changed volume:
    void setVolume(float);
    //
    void playTrackByIndex(uint32_t);
    //
    void sendPlayMessage(uint32_t id);
    //
    void togglePause();
    //
    void play();
    //
    void stop();
    //
    void playNext();
    //
    void playPrev();
    //
    void changePlaylist(int);
    //
    void movePlaylist(int plt, int before);
    //
    void newPlaylist(QString *);
    //
    void renamePlaylist(int plt, QString *name);
    //
    void setShuffle(ddb_shuffle_t);
    //
    void setRepeat(ddb_repeat_t);
};

class DBWidgetInfo {
public:
    QString internalName;
    QString friendlyName;
    DB_plugin_t *plugin;

    enum DBWidgetType {
        TypeDummy = 0,
        TypeWidgetToolbar   = 1<<0,
        TypeToolbar         = 1<<1,
        TypeDockable        = 1<<2,
        TypeMainWidget      = 1<<3
    };
    DBWidgetType type;
    // widget constructor (for DBWidgetType::TypeWidgetToolbar or DBWidgetType::TypeMainWidget)
    QWidget *(*constructor)(QWidget *parent, DBApi *api);


    // should not be used anymore
    // optional, if you want to control toolbar, constructor is ommited then
    QToolBar *(*constructorToolbar)(QWidget *parent, DBApi *api);
    QDockWidget *(*constructorDockWidget)(QWidget *parent, DBApi *api);

};

class DBWidget {

public:
    DBWidget(QWidget *parent = nullptr, DBApi *api_a = nullptr);
    ~DBWidget();
    DBApi *api;
};

typedef struct DB_qtgui_s {
    DB_gui_t gui;
    int (*register_widget) (DBWidgetInfo *);
} DB_qtgui_t;

#endif // DBAPIWRAPPER_H
