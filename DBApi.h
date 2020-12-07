// This file defines class that allows interraction with deadbeef
// TODO

#ifndef DBAPI_H
#define DBAPI_H

#include <deadbeef/deadbeef.h>

#include <QObject>
#include <QWidget>
#include <QFuture>
#include <QImage>
#include <QMimeData>
#include <QMenuBar>

// DBApi version
#define DBAPI_VMAJOR 0
#define DBAPI_VMINOR 1

class DBApi : public QObject {
    Q_OBJECT
public:
    DBApi(QObject *parent = nullptr, DB_functions_t *Api = nullptr);
    ~DBApi();

    // DBApi compatibility version
    const char DBApi_vmajor = DBAPI_VMAJOR;
    const char DBApi_vminor = DBAPI_VMINOR;

    // Access to deadbeef functions
    DB_functions_t *deadbeef = nullptr;

    //// CoverArt
    // use this function to ensure cover art plugin is available before using any functions below
    bool isCoverArtPluginAvailable();
    // load CoverArt
    QFuture<QImage *> loadCoverArt(const char *fname, const char *artist, const char *album);
    QFuture<QImage *> loadCoverArt(DB_playItem_t *);
    QImage *getDefaultCoverArt();
    // Call after you are done with cover (not if used signal cover)
    void coverArt_unref(QImage *);

    // Misc functions
    bool isPaused();
    void addTracksByUrl(const QUrl &url, int position = -1);
    float getVolume();
    ddb_playback_state_t getOutputState();
    ddb_playback_state_t getInternalState();

    // plugin message handler
    // TODO consider making it private/protected?
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);

    // Playlist
    QString const& playlistNameByIdx(int idx);
    unsigned long getPlaylistCount();

    // Menus
    void playItemContextMenu(QWidget *w, QPoint p);

    // MenuBar
    // TODO make use actions;
    QMenuBar *getMainMenuBar();
    QMenu *getMenu(const char *menu);

    // Mimedata

    QMimeData *mime_playItems(QList<DB_playItem_t *> playItems);
    QList<DB_playItem_t *> mime_playItems(const QMimeData *playItems);
    // Settings
    void confSetValue(const QString &plugname, const QString &key, const QVariant &value);
    QVariant confGetValue(const QString &plugname, const QString &key, const QVariant &defaultValue);
    void autoSetValue(void *widget, const QString &key, const QVariant &value);
    QVariant autoGetValue(void *widget, const QString &key, const QVariant &defaultValue);

    
    // Translate
    const char *_(const char *);
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
    // Queue
    void queueChanged();

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
    void *coverart_cache;
    ddb_playback_state_t internal_state;
    void *qt_settings;
    void *action_manager;

    QStringList playlistNames;

    int playlist_internal = -1;

    ddb_repeat_t currRepeat;
    ddb_shuffle_t currShuffle;

    int queue_count = 0;
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
        TypeMainWidget      = 1<<1,
        TypeWindow          = 1<<2
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
    //
    virtual QMimeData *cut();
    virtual QMimeData *copy();
    virtual void paste(const QMimeData *, QPoint);
    virtual bool canCopy(void);
    virtual bool canPaste(const QMimeData *);
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

#define DBAPI (this->api->deadbeef)
#define CONFGET(X, Y) (this->confGetValue(_internalNameWidget, X,Y))
#define CONFSET(X, Y) (this->confSetValue(_internalNameWidget, X,Y))
#define _(X) (this->api->_(X))

#endif // DBAPI_H
