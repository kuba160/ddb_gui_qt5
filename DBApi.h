// This file defines class that allows interraction with deadbeef
// TODO

#ifndef DBAPIWRAPPER_H
#define DBAPIWRAPPER_H

#include <deadbeef/deadbeef.h>

#include <QUrl>
#include <QObject>
#include <QToolBar>
#include <QtGuiSettings.h>

//#define DBAPI (this->api->deadbeef)

#define DBAPI deadbeef_internal

class DBApi : public QObject {

    Q_OBJECT

public:
    DBApi(QWidget *parent = nullptr, DB_functions_t *Api = nullptr);
    ~DBApi();

    DB_functions_t *deadbeef = nullptr;
    bool isPaused();


    void addTracksByUrl(const QUrl &url, int position = -1);

    int getVolume();

    ddb_playback_state_t getOutputState();
    ddb_playback_state_t getInternalState();

    // plugin message handler
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);
    
private:
    ddb_playback_state_t internal_state;
    QtGuiSettings *qt_settings;

// Signals are subscribed by different parts of gui
signals:
    void volumeChanged(int);
    void playlistChanged();
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void playbackPaused();
    void playbackUnPaused();
    void playbackStarted();
    void playbackStopped();
    void deadbeefActivated();

// Slots redirect messages from qt gui to deadbeef internal system
public slots:
    // When user changed volume:
    void setVolume(int);
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
};

class DBWidgetInfo {
public:
    QString internalName;
    QString friendlyName;
    DB_plugin_t *plugin;
    bool isToolbar;
    bool toolbarConstructor;
    QWidget *(*constructor)(QWidget *parent, DBApi *api);
    // optional, if you want to control toolbar, constructor is ommited then
    QToolBar *(*constructorToolbar)(QWidget *parent, DBApi *api);
};

class DBToolbarWidget {

public:
    DBToolbarWidget(QWidget *parent = nullptr, DBApi *api = nullptr);
    ~DBToolbarWidget();

    DBApi *api;
    DBWidgetInfo info;
    void loadConfig(QObject *settings);
    void saveConfig(QObject *settings);

    //virtual DBToolbarWidget *factory();
};

typedef struct DB_qtgui_s {
    DB_gui_t gui;
    int (*register_widget) (DBWidgetInfo *);
} DB_qtgui_t;

#endif // DBAPIWRAPPER_H
