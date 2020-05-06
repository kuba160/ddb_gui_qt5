TEMPLATE = lib
TARGET = ddb_gui_qt5
CONFIG += no_plugin_name_prefix plugin

SOURCES  =          include/callbacks.cpp \
                    DBApi.cpp \
		    include/parser.cpp \
		    include/qticonloader.cpp \
		    preferencesWidgets/InterfacePreferencesWidget.cpp \
		    preferencesWidgets/NetworkPreferencesWidget.cpp \
		    preferencesWidgets/PluginsPreferencesWidget.cpp \
		    preferencesWidgets/PluginSettingsWidget.cpp \
		    preferencesWidgets/SoundPreferencesWidget.cpp \
		    plugins/CoverArt/CoverArtWidget.cpp \
		    plugins/CoverArt/CoverArtWrapper.cpp \
		    plugins/CoverArt/CoverArtCache.cpp \
		    plugins/Hotkeys/HotkeyReadDialog.cpp \
		    plugins/Hotkeys/HotkeysTreeWidget.cpp \
		    plugins/Hotkeys/HotkeysWidget.cpp \
		    AboutDialog.cpp \
		    DBFileDialog.cpp \
		    GuiUpdater.cpp \
		    MainWindow.cpp \
                    MainWindowActions.cpp \
		    PlayList.cpp \
		    PlayListModel.cpp \
		    PlayListWidget.cpp \
		    PluginLoader.cpp     \
		    PreferencesDialog.cpp \
		    QtGui.cpp \
		    QtGuiSettings.cpp \
		    SeekSlider.cpp \
		    SystemTrayIcon.cpp \
		    TabBar.cpp \
		    VolumeSlider.cpp


HEADERS =               preferencesWidgets/InterfacePreferencesWidget.h \
                        DBApi.h \
			preferencesWidgets/NetworkPreferencesWidget.h \
			preferencesWidgets/PluginsPreferencesWidget.h \
			preferencesWidgets/PluginSettingsWidget.h \
			preferencesWidgets/SoundPreferencesWidget.h \
			plugins/CoverArt/CoverArtWrapper.h \
			plugins/CoverArt/CoverArtWidget.h \
			plugins/CoverArt/CoverArtCache.h \
                        plugins/Hotkeys/HotkeyReadDialog.h \
                        plugins/Hotkeys/HotkeysTreeWidget.h \
                        plugins/Hotkeys/HotkeysWidget.h \
			AboutDialog.h \
			DBFileDialog.h \
			GuiUpdater.h \
			MainWindow.h \
			PlayList.h \
			PlayListModel.h \
			PlayListWidget.h \
			PluginLoader.h \
			PreferencesDialog.h \
			QtGuiSettings.h \
			SeekSlider.h \
			SystemTrayIcon.h \
			TabBar.h \
                        VolumeSlider.h

FORMS =         preferencesWidgets/InterfacePreferencesWidget.ui \
		preferencesWidgets/NetworkPreferencesWidget.ui \
		preferencesWidgets/PluginsPreferencesWidget.ui \
		preferencesWidgets/SoundPreferencesWidget.ui \
		AboutDialog.ui \
		MainWindow.ui

RESOURCES = Resources.qrc

QT += gui widgets concurrent

DEFINES += ARTWORK_ENABLED HOTKEYS_ENABLED "DEADBEEF_PREFIX=\\\"donotuse\\\""

INCLUDEPATH += "./include"
