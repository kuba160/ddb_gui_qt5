TEMPLATE = lib
TARGET = ddb_gui_qt5
CONFIG += no_plugin_name_prefix plugin
INCLUDEPATH += "./include"

SOURCES  =  include/callbacks.cpp \
            include/parser.cpp \
            include/qticonloader.cpp \
            plugins/Hotkeys/HotkeyReadDialog.cpp \
            plugins/Hotkeys/HotkeysTreeWidget.cpp \
            plugins/Hotkeys/HotkeysWidget.cpp \
            preferencesWidgets/InterfacePreferencesWidget.cpp \
            preferencesWidgets/NetworkPreferencesWidget.cpp \
            preferencesWidgets/PluginsPreferencesWidget.cpp \
            preferencesWidgets/PluginSettingsWidget.cpp \
            preferencesWidgets/SoundPreferencesWidget.cpp \
            widgets/CoverArt.cpp \
            widgets/PlaybackButtons.cpp \
            widgets/Playlist.cpp \
            widgets/PlaylistBrowser.cpp \
            widgets/SeekSlider.cpp \
            widgets/TabBar.cpp \
            widgets/VolumeSlider.cpp \
            AboutDialog.cpp \
            CoverArtCache.cpp \
            DBApi.cpp \
            DBFileDialog.cpp \
            DeadbeefTranslator.cpp \
            DefaultPlugins.cpp \
            GuiUpdater.cpp \
            MainWindow.cpp \
            MainWindowActions.cpp \
            PlayListModel.cpp \
            PluginLoader.cpp     \
            PreferencesDialog.cpp \
            QtGui.cpp \
            QtGuiSettings.cpp \
            SystemTrayIcon.cpp

HEADERS =   plugins/Hotkeys/HotkeyReadDialog.h \
            plugins/Hotkeys/HotkeysTreeWidget.h \
            plugins/Hotkeys/HotkeysWidget.h \
            preferencesWidgets/InterfacePreferencesWidget.h \
            preferencesWidgets/NetworkPreferencesWidget.h \
            preferencesWidgets/PluginsPreferencesWidget.h \
            preferencesWidgets/PluginSettingsWidget.h \
            preferencesWidgets/SoundPreferencesWidget.h \
            widgets/CoverArt.h \
            widgets/PlaybackButtons.h \
            widgets/Playlist.h \
            widgets/PlaylistBrowser.h \
            widgets/SeekSlider.h \
            widgets/TabBar.h \
            widgets/VolumeSlider.h \
            AboutDialog.h \
            CoverArtCache.h \
            DBApi.h \
            DBFileDialog.h \
            DeadbeefTranslator.h \
            DefaultPlugins.h \
            GuiUpdater.h \
            MainWindow.h \
            PlayListModel.h \
            PluginLoader.h \
            PreferencesDialog.h \
            QtGuiSettings.h \
            SystemTrayIcon.h

FORMS =     preferencesWidgets/InterfacePreferencesWidget.ui \
            preferencesWidgets/NetworkPreferencesWidget.ui \
            preferencesWidgets/PluginsPreferencesWidget.ui \
            preferencesWidgets/SoundPreferencesWidget.ui \
            AboutDialog.ui \
            MainWindow.ui

RESOURCES = Resources.qrc

QT += gui widgets concurrent
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
