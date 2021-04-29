/*
    ddb_gui_qt5 - Qt user interface
    Copyright (C) 2010 Anton Novikov <tonn.post@gmail.com>
    Copyright (C) 2011 Semen Minyushov <semikmsv@gmail.com>
    Copyright (C) 2013 Karjavin Roman <redpunk231@gmail.com>
    Copyright (C) 2019-2020 Jakub Wasylków <kuba_160@protonmail.com>

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


#include <unistd.h>
#include <QApplication>

#include "DBApi.h"
#include "MainWindow.h"
#include "QtGuiSettings.h"
#include <QStyleFactory>
#include <QLocale>

#include "PluginLoader.h"
#include "DeadbeefTranslator.h"

#undef DBAPI
#define DBAPI deadbeef_internal

static int pluginStart();
static int pluginStop();
static int pluginConnect();

DB_functions_t *deadbeef_internal;
DB_qtgui_t qt_plugin;
DB_gui_t &plugin = qt_plugin.gui;
// TODO make casual plugin list
DB_artwork_plugin_t *coverart_plugin;


DBApi *api = nullptr;
PluginLoader *pl = nullptr;
MainWindow *w = nullptr;
DeadbeefTranslator *tr = nullptr;
QApplication *app = nullptr;

static int pluginMessage_wrapper(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    return api->pluginMessage(id, ctx, p1, p2);
}

static int initializeApi() {
    if (!api) {
        while (!deadbeef_internal) {
            usleep(10000);
        }

        // provide dummy args for QApplication
        static char argv0[] = "a.out";
        static char *argv[] = {argv0, nullptr};
        static int argc = sizeof(argv) / sizeof(char*) - 1;
        app = new QApplication(argc, argv);
        QApplication::setOrganizationName("deadbeef");
        QApplication::setApplicationName("DeaDBeeF");
        dbtr = new DeadbeefTranslator(app);
        app->installTranslator(dbtr);

        //QApplication::setStyle(QStyleFactory::create("breeze"));

    #ifdef __MINGW32__
        QStringList theme_search_paths = QIcon::themeSearchPaths();
        theme_search_paths.append("./share/icons");
        QIcon::setThemeSearchPaths(theme_search_paths);
        qDebug() << QIcon::themeSearchPaths();
        //QIcon::setThemeName("Windows-10-Icons");
        QIcon::setThemeName("Adwaita");
    #endif

        // setup settings
        QString file = QString("%1/%2") .arg(deadbeef_internal->get_system_dir(DDB_SYS_DIR_CONFIG), "qt5");
        QtGuiSettings::setDefaultFormat(QSettings::IniFormat);
        QtGuiSettings::setPath(QSettings::IniFormat, QSettings::UserScope, file);


        if (!pl) {
            pl = new PluginLoader();
        }
        api = new DBApi(app, deadbeef_internal);
        // initialize window
        w = new MainWindow(nullptr, api);
        plugin.plugin.message = pluginMessage_wrapper;
    }
    return 0;
}

static int initializePluginLoader() {
    if (!pl) {
        pl = new PluginLoader();
    }
    if (!api) {
        initializeApi();
    }
    return 0;
}

static int pluginStop() {
    QApplication::instance()->quit();
    qDebug() << "QtGui_stop completed";
    return 0;
}
static int pluginConnect() {
    return 0;
}

static int registerWidget (DBWidgetInfo *info) {
    initializeApi();
    pl->widgetLibraryAppend(info);
    return 0;
}


static int pluginStart() {
    initializeApi();


    w->loadConfig();
    w->show();
    app->exec();

    // shutdown
    delete w;
    delete api;
    delete pl;
    delete app;

    DBAPI->sendmessage(DB_EV_TERMINATE, 0, 0, 0);
    return 0;
}

extern "C" {
    DB_plugin_t *ddb_gui_qt5_load(DB_functions_t *dbapi) {
        deadbeef_internal = dbapi;
        plugin.plugin.api_vmajor = 1;
        plugin.plugin.api_vminor = 9;
        plugin.plugin.version_major = 1;
        plugin.plugin.version_minor = 9;
        plugin.plugin.type = DB_PLUGIN_GUI;
        plugin.plugin.id = "qt5";
        plugin.plugin.name = "Qt user interface";
        plugin.plugin.descr = "Qt user interface";
        plugin.plugin.copyright =
            "ddb_gui_qt5 - Qt user interface\n"
            "Copyright (C) 2010 Anton Novikov <tonn.post@gmail.com>\n"
            "Copyright (C) 2011 Semen Minyushov <semikmsv@gmail.com>\n"
            "Copyright (C) 2013 Karjavin Roman <redpunk231@gmail.com>\n"
            "Copyright (C) 2019-2021 Jakub Wasylków <kuba_160@protonmail.com>\n"
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
        plugin.plugin.message = nullptr;
        qt_plugin.register_widget = registerWidget;
        return DB_PLUGIN(&plugin);
    }
}
