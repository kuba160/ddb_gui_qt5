#ifndef QTGUI_H
#define QTGUI_H

#include <QDebug>

#include <deadbeef/deadbeef.h>

#include "DBApi.h"
#include "PluginLoader.h"
#include "MainWindow.h"

#include "include/artwork.h"

#define PLUGIN plugin
//#define DBAPI deadbeef

extern DB_functions_t *deadbeef_internal;
extern DB_qtgui_t qt_plugin;
extern DB_gui_t &plugin;

extern DBApi *api;
extern PluginLoader *pl;
//extern MainWindow *w;

#endif // QTGUI_H
