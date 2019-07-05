/*
    ddb_gui_qt5 - Qt user interface
    Copyright (C) 2010 Anton Novikov <tonn.post@gmail.com>
    Copyright (C) 2011 Semen Minyushov <semikmsv@gmail.com>
    Copyright (C) 2013 Karjavin Roman <redpunk231@gmail.com>
    Copyright (C) 2019 Jakub Wasylków <kuba_160@protonmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "QtGui.h"

#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include <QLocale>

#include "DBApiWrapper.h"
#include "MainWindow.h"
#include "QtGuiSettings.h"

static int pluginStart();
static int pluginStop();
static int pluginConnect();
static int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);
void MainThreadRun(void *);

static intptr_t thread;

DB_functions_t *deadbeef;
DB_gui_t plugin;

#ifdef HOTKEYS_ENABLED
DB_hotkeys_plugin_t *hotkeys_plugin;
#endif
#ifdef ARTWORK_ENABLED
DB_artwork_plugin_t *coverart_plugin;
#endif

static int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    switch (id) {
    case DB_EV_SONGCHANGED:
        WRAPPER->onSongChanged((ddb_event_trackchange_t *)ctx);
        break;
    case DB_EV_PAUSED:
        WRAPPER->onPause(p1);
        break;
    case DB_EV_PLAYLISTCHANGED:
        WRAPPER->onPlaylistChanged();
        break;
    case DB_EV_ACTIVATED:
        WRAPPER->onDeadbeefActivated();
        break;
    }
    return 0;
}

static int pluginStart() {
    MainThreadRun (NULL);
    return 0;
}

static int pluginStop() {
    QApplication::instance()->quit();
    qDebug() << "QtGui_stop completed";
    return 0;
}
static int pluginConnect() {
#ifdef ARTWORK_ENABLED
    coverart_plugin = (DB_artwork_plugin_t *)DBAPI->plug_get_for_id("artwork");
    if (coverart_plugin)
        qDebug() << "qtui: found cover-art plugin";

#endif

#ifdef HOTKEYS_ENABLED
    hotkeys_plugin = (DB_hotkeys_plugin_t *)DBAPI->plug_get_for_id("hotkeys");
    if (hotkeys_plugin)
        qDebug() << "qtui: found global hotkeys plugin";
#endif
    return 0;
}


void MainThreadRun(void *) {
    // provide dummy args
    char argv0[] = "a.out";
    char *argv[] = {argv0, nullptr};
    int argc = sizeof(argv) / sizeof(char*) - 1;
    QApplication app(argc, argv);
    QApplication::setOrganizationName("deadbeef");
    QApplication::setApplicationName("DeaDBeeF");

    QString locale = QLocale::system().name();

    //QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    MainWindow w;
    w.show();
    app.exec();
}

extern "C" {
    DB_plugin_t *ddb_gui_qt5_load(DB_functions_t *api) {
        deadbeef = api;
        plugin.plugin.api_vmajor = 1;
        plugin.plugin.api_vminor = 9;
        plugin.plugin.version_major = 1;
        plugin.plugin.version_minor = 0;
        plugin.plugin.type = DB_PLUGIN_GUI;
        plugin.plugin.id = "qt5";
        plugin.plugin.name = "Qt user interface";
        plugin.plugin.descr = "Qt user interface";
        plugin.plugin.copyright =
            "ddb_gui_qt5 - Qt user interface\n"
            "Copyright (C) 2010 Anton Novikov <tonn.post@gmail.com>\n"
            "Copyright (C) 2011 Semen Minyushov <semikmsv@gmail.com>\n"
            "Copyright (C) 2013 Karjavin Roman <redpunk231@gmail.com>\n"
            "Copyright (C) 2019 Jakub Wasylków <kuba_160@protonmail.com>\n"
            "\n"
            "This program is free software; you can redistribute it and/or\n"
            "modify it under the terms of the GNU General Public License\n"
            "as published by the Free Software Foundation; either version 2\n"
            "of the License, or (at your option) any later version.\n"
            "\n"
            "This program is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
            "GNU General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU General Public License\n"
            "along with this program; if not, write to the Free Software\n"
            "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n";
        plugin.plugin.website = "https://github.com/kuba160/ddb_gui_qt5";
        plugin.plugin.start = pluginStart;
        plugin.plugin.stop = pluginStop;
        plugin.plugin.connect = pluginConnect;
        plugin.plugin.message = pluginMessage;
        return DB_PLUGIN(&plugin);
    }
}

