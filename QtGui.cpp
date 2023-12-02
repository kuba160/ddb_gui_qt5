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

#if USE_WIDGETS
#include <QApplication>
#include "widgets/MainWindow.h"
#else
#include <QGuiApplication>
#include "PluginLoader.h"
#endif

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>

#include <QLocale>
#include <QQuickWindow>


//#include "DeadbeefTranslator.h"


#define DBAPI (deadbeef_internal)

static int pluginStart();
static int pluginStop();
static int pluginConnect();

DB_functions_t *deadbeef_internal;
DB_qtgui_t qt_plugin;
DB_gui_t &plugin = qt_plugin.gui;
// TODO make casual plugin list

DBApi *api = nullptr;
QQmlApplicationEngine *engine = nullptr;
PluginLoader *pl = nullptr;
//DeadbeefTranslator *tr = nullptr;*/
#if USE_WIDGETS
MainWindow *w = nullptr;
QApplication *app = nullptr;
#else
QGuiApplication *app = nullptr;
#endif


static int pluginMessage_wrapper(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    return api->pluginMessage(id, ctx, p1, p2);
}

static void initializeQApp() {
    qmlRegisterUncreatableType<PlaybackControl>("DBApi", 1, 0, "Playback","");

    // provide dummy args for QApplication
    static char argv0[] = "a.out";
    static char *argv[] = {argv0, nullptr};
    static int argc = sizeof(argv) / sizeof(char*) - 1;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#if USE_WIDGETS
app = new QApplication(argc, argv);
#else
app = new QGuiApplication(argc, argv);
#endif

    //QGuiApplication::setOrganizationName("deadbeef");
    QGuiApplication::setApplicationName("qt5");
    app->setWindowIcon(QIcon(":/images/deadbeef.png"));

#ifdef __MINGW32__
    QStringList theme_search_paths = QIcon::themeSearchPaths();
    theme_search_paths.append("./share/icons");
    QIcon::setThemeSearchPaths(theme_search_paths);
    qDebug() << QIcon::themeSearchPaths();
    //QIcon::setThemeName("Windows-10-Icons");
    QIcon::setThemeName("Adwaita");

    // Set opengl api for qt quick (qt 6 and higher)
    #if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    #endif
#endif

    // fix for lag on linux qquick
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setSwapInterval(0);
    QSurfaceFormat::setDefaultFormat(format);

}

static void initializeTranslator() {
    /// TODO
    /*dbtr = new DeadbeefTranslator(app);
    app->installTranslator(dbtr);*/
}

static void startWidgets() {
#if USE_WIDGETS
    w = new MainWindow(nullptr, api);
    w->show();

#endif
}

static void unloadWidgets() {
#if USE_WIDGETS
    delete w;
#endif
}

static void startQuick() {
    engine = new QQmlApplicationEngine();
    //engine->addImportPath(":/qt/qml");

    const QUrl url(u"qrc:/qt/qml/DeaDBeeF/Q/DDB2/main.qml"_qs);
    //const QUrl url(QStringLiteral("qrc:/qml/DDB2/main.qml"));

#if USE_WIDGETS
    engine->rootContext()->setContextProperty("app", app);
#endif
    /*engine->rootContext()->setContextProperty("api", api);
    engine->rootContext()->setContextProperty("actions", &api->actions);
    engine->rootContext()->setContextProperty("playback", &api->playback);
    engine->rootContext()->setContextProperty("conf", &api->conf);
    engine->rootContext()->setContextProperty("cover", (CoverArt*)(&api->playlist));
    engine->rootContext()->setContextProperty("playlist", &api->playlist);
    engine->rootContext()->setContextProperty("eq", &api->eq);*/
    engine->rootContext()->setContextProperty("_db_bg_override", false);
    engine->rootContext()->setContextProperty("_db_bg", "transparent");
    engine->rootContext()->setContextProperty("_db_do_not_load", false);
    engine->rootContext()->setContextProperty("_qt_version", QT_VERSION_STR);

    pl = new PluginLoader(nullptr);
    engine->rootContext()->setContextProperty("plugin", pl);

    engine->load(url);
}

static void unloadQuick() {
    if (engine) {
        delete engine;
        delete pl;
    }
}

static int pluginStop() {
    QGuiApplication::instance()->quit();
    qDebug() << "QtGui_stop completed";
    return 0;
}
static int pluginConnect() {
    qDebug() << "CONNECT";
    return 0;
}

static int registerWidget (WidgetPluginConstructor constructor) {
    if (!api) {
        return 1;
    }
    //pl->widgetLibraryAppend(info);
    return 0;
}

static int pluginStart() {
    initializeQApp();
    api = new DBApi(nullptr, deadbeef_internal);
    plugin.plugin.message = pluginMessage_wrapper;

    startWidgets();
    startQuick();

    // GUI thread loop
    app->exec();

    unloadQuick();
    unloadWidgets();

    delete api;
    delete app;

    DBAPI->sendmessage(DB_EV_TERMINATE, 0, 0, 0);
    return 0;
}

extern "C" {
    DB_plugin_t *ddb_gui_qt5_load(DB_functions_t *Api) {
        deadbeef_internal = Api;
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
