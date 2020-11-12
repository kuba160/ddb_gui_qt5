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
#include "CoverArtCache.h"

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

    //// CoverArt
    // functions might return invalid value if plugin is unavailable
    bool isCoverArtPluginAvailable();
    // load CoverArt
    QFuture<QImage *> loadCoverArt(const char *fname, const char *artist, const char *album);
    QFuture<QImage *> loadCoverArt(DB_playItem_t *);
    QImage *getDefaultCoverArt();
    // Call after you are done with cover (not if used signal cover)
    void coverArt_unref(QImage *);


    bool isPaused();
    void addTracksByUrl(const QUrl &url, int position = -1);
    float getVolume();
    ddb_playback_state_t getOutputState();
    ddb_playback_state_t getInternalState();

    // plugin message handler
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);


    QString const& playlistNameByIdx(int idx);
    unsigned long getPlaylistCount();

    // Settings
    void confSetValue(const QString &plugname, const QString &key, const QVariant &value);
    QVariant confGetValue(const QString &plugname, const QString &key, const QVariant &defaultValue);
    void autoSetValue(void *widget, const QString &key, const QVariant &value);
    QVariant autoGetValue(void *widget, const QString &key, const QVariant &defaultValue);
    
// Signals are subscribed by different parts of gui
signals:
    // Volume
    void volumeChanged(float);
    // Playback
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void playbackPaused();
    void playbackUnPaused();
    void playbackStarted();
    void playbackStopped();
    // Playlist
    void playlistChanged();
    void playlistChanged(int to);
    void playlistMoved(int plt, int before);
    void playlistCreated();
    void playlistRenamed(int plt);
    // DeaDBeeF Window
    void deadbeefActivated();
    // Shuffle/Repeat
    void shuffleChanged();
    void repeatChanged();
    // Cover
    void currCoverChanged(QImage *);

// Slots redirect messages from qt gui to deadbeef internal system
public slots:
    // Change volume
    void setVolume(float);
    // Just send message (id only)
    void sendPlayMessage(uint32_t id);
    // Playback
    void togglePause();
    void play();
    void playNext();
    void playPrev();
    void playTrackByIndex(uint32_t);
    void stop();
    // Playlist
    void changePlaylist(int);
    void movePlaylist(int plt, int before);
    void newPlaylist(QString *);
    void renamePlaylist(int plt, const QString *name);
    // Shuffle/Repeat
    void setShuffle(ddb_shuffle_t);
    void setRepeat(ddb_repeat_t);

private:
    CoverArtCache coverart_cache;
    ddb_playback_state_t internal_state;
    QtGuiSettings *qt_settings;

    QStringList playlistNames;

    int playlist_internal = -1;

    ddb_repeat_t currRepeat;
    ddb_shuffle_t currShuffle;
private slots:
    void onCurrCoverChanged();
};

class DBWidgetInfo {
public:
    QString internalName;
    QString friendlyName;
    DB_plugin_t *plugin;

    enum DBWidgetType {
        TypeDummy = 0,
        TypeToolbar         = 1<<0,
        TypeMainWidget      = 1<<1
    };
    // Toolbar or MainWidget(dockable)
    DBWidgetType type;
    // widget constructor (save to cast parent to QDockWidget* for Toolbar)
    QWidget *(*constructor)(QWidget *parent, DBApi *api);
};

class DBWidget {

public:
    DBWidget(QWidget *parent = nullptr, DBApi *api_a = nullptr);
    ~DBWidget();
    DBApi *api;
    QString _internalNameWidget;
};

typedef struct DB_qtgui_s {
    DB_gui_t gui;
    int (*register_widget) (DBWidgetInfo *);
} DB_qtgui_t;

class playItemList {
public:
    qint32 count;
    QList<DB_playItem_t *> list;
};

QDataStream &operator<<(QDataStream &ds, const playItemList &pil);
QDataStream &operator>>(QDataStream &ds, playItemList &pil);

#endif // DBAPIWRAPPER_H
