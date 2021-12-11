// This file defines class that allows interraction with deadbeef
// TODO

#ifndef DBAPI_H
#define DBAPI_H

#include <deadbeef/deadbeef.h>

#include <QtGlobal>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#define ENDL Qt::endl
#else
#define ENDL endl
#endif

#include <QObject>
#include <QWidget>
#include <QFuture>
#include <QImage>
#include <QMimeData>
#include <QMenuBar>
#include <QClipboard>

// DBApi version
#define DBAPI_VMAJOR 0
#define DBAPI_VMINOR 5

typedef QList<DB_playItem_t *> playItemList;

QDataStream &operator<<(QDataStream &ds, const playItemList &pil);
QDataStream &operator>>(QDataStream &ds, playItemList &pil);


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
    // check if coverart is in cache, true: means you can call getCoverArt and get the cover
    // NOTE: this function returns true even if there is no cover for specific track
    bool isCoverArtCached(DB_playItem_t *, QSize size = QSize());
    // load cover that is not in cache, QImage returned via QFuture has to be unref'd later
    // if you set up size, the cover will be scaled
    // use scaling if you need many covers of specific size, don't use for widget scaling etc.
    QFuture<QImage *> requestCoverArt(DB_playItem_t *, QSize size = QSize());
    // get cached cover art, nullptr if not cached (use requestCoverArt to cache it)
    QImage * getCoverArt(DB_playItem_t *, QSize size = QSize());
    // default cover art
    QImage * getCoverArtDefault();
    // Call after you are done with cover
    void coverArt_unref(QImage *);
    void coverArt_ref(QImage *);
    // Call if track becomes no longer accessible
    void coverArt_track_unref(DB_playItem_t *);


    // Misc functions
    bool isPaused();
    void addTracksByUrl(const QUrl &url, int position = -1);

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
    void playlistContextMenu(QWidget *w, QPoint p, int plt);

    // MenuBar
    // TODO make use actions;
    QMenuBar *getMainMenuBar();
    QMenu *getMenu(const char *menu);

    // Mimedata

    QMimeData *mime_playItems(QList<DB_playItem_t *> playItems);
    QMimeData *mime_playItemsCopy(QList<DB_playItem_t *> playItems);
    QList<DB_playItem_t *> mime_playItems(const QMimeData *playItems);
    QList<DB_playItem_t *> mime_playItemsCopy(const QMimeData *playItems);
    // Settings
    void confSetValue(const QString &plugname, const QString &key, const QVariant &value);
    QVariant confGetValue(const QString &plugname, const QString &key, const QVariant &defaultValue);

    // Clipboard access
    QClipboard *clipboard;
    void clearClipboard();
    
    // Translate
    const char *_(const char *);
// Signals are subscribed by different parts of gui
signals:
    // Playback
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void trackChanged();
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
    void playlistRemoved(int plt);
    void playlistContentChanged(ddb_playlist_t *plt);
    // DeaDBeeF Window
    void deadbeefActivated();
    // Shuffle/Repeat
    void shuffleChanged();
    void repeatChanged();
    // Queue
    void queueChanged();
    // Specific actions triggered by user
    void jumpToCurrentTrack();
    void selectionChanged();

// Slots redirect messages from qt gui to deadbeef internal system
public slots:
    // Volume
    virtual void setVolume(int);
    // Just send message (id only)
    virtual void sendPlayMessage(uint32_t id);
    // Playback
    virtual void togglePause();
    virtual void play();
    virtual void playNext();
    virtual void playPrev();
    virtual void playTrackByIndex(uint32_t);
    virtual void stop();
    // Playlist
    virtual void changePlaylist(int);
    virtual void movePlaylist(int plt, int before);
    virtual void newPlaylist(QString);
    virtual void renamePlaylist(int plt, const QString *name);
    virtual void renamePlaylist(int plt); // Dialog
    virtual void removePlaylist(int plt);
    virtual void clearPlaylist(int plt);
    virtual void loadPlaylist(const QString &fname);
    // use if playItems are on same playlist
    virtual void removeTracks(playItemList list);

    // Shuffle/Repeat
    virtual void setShuffle(ddb_shuffle_t);
    virtual void setRepeat(ddb_repeat_t);

private:
    void *coverart_cache = nullptr;
    ddb_playback_state_t internal_state;
    void *qt_settings;
    void *action_manager;

    QStringList playlistNames;

    int playlist_internal = -1;

    ddb_repeat_t currRepeat;
    ddb_shuffle_t currShuffle;

    int queue_count = 0;

    /// Qt quick section

    // internal volume
    float m_volume;
    //
public:
    // Volume (in dB)
    Q_PROPERTY(float volume READ getVolume WRITE setVolume NOTIFY volumeChanged)

public slots:
    // Volume
    virtual float getVolume();
    virtual void setVolume(float);

signals:
    void volumeChanged(float);

};

class DBWidgetInfo {
public:
    QString internalName;
    QString friendlyName;
    DB_plugin_t *plugin;

    enum DBWidgetType {
        TypeDummy           = 0,
        TypeToolbar         = 1<<0,
        TypeMainWidget      = 1<<1,
        TypeStatusBar       = 1<<2
    };
    // Type
    DBWidgetType type;
    // widget constructor (save to cast parent to (QToolbar * / QDockWidget *) for Toolbar / MainWindow[if in dock])
    QWidget *(*constructor)(QWidget *parent, DBApi *api);
    // Parent Properties:
    // - FriendlyName - name (QString)
    // - InternalName - internal name (QString)
    // - DesignMode - design mode enabled (bool)
    // POSSIBLE FUTURE Properties:
    // - Stylesheet - user-entered stylesheet for plugin (QString)
    // - StylesheetOverride - disable widget stylesheet (bool)
    // - DBApi - DBApi pointer (qintptr), could remove DBApi argument requirement
    // - WidgetType - one of DBWidgetTypes
    // Children Properties (the one you are creating):
    // - Actions - QGroupActions pointer with actions for right click menu (TODO: maybe separate playlist with playitem actions?)
    //   - Playqueue: add_to_playback_queue, remove_from_playback_queue
    //   - Clipboard: cut, copy, paste
    //   - Delete: delete
    //   - Track properties: track_properties
    // - playItemsSelected - used for Actions, int of how many tracks are selected
};

class DBWidget {
public:
    DBWidget(QWidget *parent = nullptr, DBApi *api_a = nullptr) {
        if (api_a == nullptr) {
            qDebug() << "Widget (" << parent <<") initialized without api pointer!";
            return;
        }
        api = api_a;
        if (parent) {
            _internalNameWidget = parent->property("internalName").toString();
        }
        if (api_a->DBApi_vmajor > DBAPI_VMAJOR || api_a->DBApi_vminor > DBAPI_VMINOR) {
            qDebug() << "WARNING:" << _internalNameWidget <<
                        QString("plugin version older than api! (%1.%2 < %3.%4)")
                        .arg(DBAPI_VMAJOR) .arg(DBAPI_VMINOR)
                        .arg(api_a->DBApi_vmajor) .arg(api_a->DBApi_vminor) << ENDL;
        }
    }
    DBApi *api;
    QString _internalNameWidget;
};

typedef struct DB_qtgui_s {
    DB_gui_t gui;
    int (*register_widget) (DBWidgetInfo *);
} DB_qtgui_t;

/// Macros to be used in DBWidget
// Pointer to deadbeef functions
#define DBAPI (this->api->deadbeef)
// Macro to read entry X with default value Y to config (instance specific, returns QVariant)
#define CONFGET(X, Y) (this->api->confGetValue(_internalNameWidget, X,Y))
// Macro to save entry X with value Y to config (instance specific, returns void)
#define CONFSET(X, Y) (this->api->confSetValue(_internalNameWidget, X,Y))

#endif // DBAPI_H
