#ifndef QTGUI_H
#define QTGUI_H

#include <deadbeef/deadbeef.h>

#include <QDebug>

#define Log(X) qDebug() << Q_FUNC_INFO << ":" << X

#include "config.h"

#ifdef ARTWORK_ENABLED
#include <plugins/CoverArt/include/artwork.h>
#define COVERART coverart_plugin
extern DB_artwork_plugin_t *coverart_plugin;
#endif

#ifdef HOTKEYS_ENABLED
#include <plugins/Hotkeys/include/hotkeys.h>
#define HOTKEYS hotkeys_plugin
extern DB_hotkeys_plugin_t *hotkeys_plugin;
#endif

#define PLUGIN plugin
#define DBAPI deadbeef

extern DB_functions_t *deadbeef;
extern DB_gui_t plugin;

#endif // QTGUI_H
