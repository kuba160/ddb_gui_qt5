TEMPLATE = lib
TARGET = ddb_gui_qt5
CONFIG += no_plugin_name_prefix plugin qt shared c++11
INCLUDEPATH += "./include"

SOURCES  =  \
            widgets/CoverArt.cpp \
            widgets/LogViewer.cpp \
            widgets/Medialib.cpp \
            widgets/PlaybackButtons.cpp \
            widgets/Playlist.cpp \
            widgets/PlaylistBrowser.cpp \
            widgets/SeekSlider.cpp \
            widgets/StatusBar.cpp \
            widgets/TabBar.cpp \
            widgets/VolumeSlider.cpp \
            widgets/QueueManager.cpp \
            AboutDialog.cpp \
            ActionManager.cpp \
            CoverArtCache.cpp \
            CoverArtBackend.cpp \
            DBApi.cpp \
            DBFileDialog.cpp \
            DeadbeefTranslator.cpp \
            DefaultActions.cpp \
            DefaultPlugins.cpp \
            MainWindow.cpp \
            MediasourceModel.cpp \
            PlayItemModel.cpp \
            PlayqueueModel.cpp \
            PlaylistBrowserModel.cpp \
            PlaylistModel.cpp \
            PlaylistView.cpp \
            PluginLoader.cpp \
            QtGui.cpp \
            QtGuiSettings.cpp \
            SystemTrayIcon.cpp

HEADERS =   widgets/CoverArt.h \
            widgets/LogViewer.h \
            widgets/Medialib.h \
            widgets/PlaybackButtons.h \
            widgets/Playlist.h \
            widgets/PlaylistBrowser.h \
            widgets/SeekSlider.h \
            widgets/StatusBar.h \
            widgets/TabBar.h \
            widgets/VolumeSlider.h \
            widgets/QueueManager.h \
            AboutDialog.h \
            ActionManager.h \
            CoverArtCache.h \
            CoverArtBackend.h \
            DBApi.h \
            DBFileDialog.h \
            DeadbeefTranslator.h \
            DefaultActions.h \
            DefaultPlugins.h \
            MainWindow.h \
            MediasourceModel.h \
            PlayItemModel.h \
            PlayqueueModel.h \
            PlaylistBrowserModel.h \
            PlaylistModel.h \
            PlaylistView.h \
            PluginLoader.h \
            QtGui.h \
            QtGuiSettings.h \
            SystemTrayIcon.h

FORMS =     AboutDialog.ui \
            DefaultActions.ui

RESOURCES = Resources.qrc

QT += gui widgets concurrent svg
DEFINES += ARTWORK_ENABLED HOTKEYS_ENABLED "DEADBEEF_PREFIX=\\\"donotuse\\\""

# link with gettext?
load(configure)
if (qtCompileTest(libintl)) {
    !build_pass:warning("includes gettext")
    DEFINES += USE_GETTEXT
    win32 {
        LIBS += -lintl
    }
}
else {
    !build_pass:warning("compiling without gettext support")
}

# install path
unix:!macx {
    target.path = ~/.local/lib/deadbeef/
    INSTALLS += target
}
