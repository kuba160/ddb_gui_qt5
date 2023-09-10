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
#include <QQmlEngine>


#include "Actions.h"
#include "Equalizer.h"
#include "PlaybackControl.h"
#include "PlaylistManager.h"
#include "Settings.h"
#include "Visualizations.h"

// DBApi version
#define DBAPI_VMAJOR 1
#define DBAPI_VMINOR 0

class DBApi;
extern DBApi *DBApi_main;

class DBApi : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    QML_UNCREATABLE("Cannot create DBApi in QML!")
public:
    static DBApi *create(QQmlEngine *, QJSEngine *) {
        QJSEngine::setObjectOwnership(DBApi_main, QJSEngine::CppOwnership);
        return DBApi_main;
    }
    Q_PROPERTY(int version_major MEMBER DBApi_vmajor CONSTANT)
    Q_PROPERTY(int version_minor MEMBER DBApi_vminor CONSTANT)

public:
    DBApi(QObject *parent, DB_functions_t *Api);
    ~DBApi();

    // DBApi compatibility version
    const char DBApi_vmajor = DBAPI_VMAJOR;
    const char DBApi_vminor = DBAPI_VMINOR;

    // Access to deadbeef functions
    DB_functions_t *deadbeef = nullptr;

    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);

    Actions actions;
    Settings conf;
    Equalizer eq;
    PlaybackControl playback;
    PlaylistManager playlist;
    Visualizations viz;


    Q_PROPERTY(Actions* actions READ getActions CONSTANT)
    Q_PROPERTY(Settings* conf READ getConf CONSTANT)
    Q_PROPERTY(Equalizer* eq READ getEq CONSTANT)
    Q_PROPERTY(PlaybackControl* playback READ getPlayback CONSTANT)
    Q_PROPERTY(Visualizations* viz READ getViz CONSTANT)
    Q_PROPERTY(PlaylistManager* playlist READ getPlaylist CONSTANT)
    public slots:
    Actions* getActions() {
        return &actions;
    }
    Settings* getConf() {
        return &conf;
    }
    Equalizer* getEq() {
        return &eq;
    }
    PlaybackControl* getPlayback() {
        return &playback;
    }
    Visualizations* getViz() {
        return &viz;
    }
    PlaylistManager* getPlaylist() {
        return &playlist;
    }
};

typedef std::function<QObject* (QWidget *parent, DBApi *Api)> WidgetPluginConstructor;


typedef struct DB_qtgui_s {
    DB_gui_t gui;
    int (*register_widget) (WidgetPluginConstructor);
    int (*register_module) (const char*);
} DB_qtgui_t;

/// Macros to be used in DBWidget
// Pointer to deadbeef functions
//#define DBAPI (this->api->deadbeef)
// Macro to read entry X with default value Y to config (instance specific, returns QVariant)
#define CONFGET(X, Y) (this->api->confGetValue(_internalNameWidget, X,Y))
// Macro to save entry X with value Y to config (instance specific, returns void)
#define CONFSET(X, Y) (this->api->confSetValue(_internalNameWidget, X,Y))


//class TrackRefc {
//public:
//    static void item_ref(DB_playItem_t *it);
//    static void item_unref(DB_playItem_t *it);
//    static void override_ref();
//};

#endif // DBAPI_H
