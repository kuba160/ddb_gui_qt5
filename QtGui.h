#ifndef QTGUI_H
#define QTGUI_H

#include <QDebug>

#include <deadbeef/deadbeef.h>

#include "DBApi.h"
#include "PluginLoader.h"
#include "MainWindow.h"

#include "include/artwork.h"


#define COVERART coverart_plugin
//extern QtPlugin_t qtCoverart;
extern DB_artwork_plugin_t *coverart_plugin;

#ifdef HOTKEYS_ENABLED
#include <plugins/Hotkeys/include/hotkeys.h>
#define HOTKEYS hotkeys_plugin
extern DB_hotkeys_plugin_t *hotkeys_plugin;
#endif

#define PLUGIN plugin
//#define DBAPI deadbeef

extern DB_functions_t *deadbeef_internal;
extern DB_qtgui_t qt_plugin;
extern DB_gui_t &plugin;

extern DBApi *api;
extern PluginLoader *pl;;
//extern MainWindow *w;

#endif // QTGUI_H
