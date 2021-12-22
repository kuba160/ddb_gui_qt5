cmake_minimum_required(VERSION 3.1.0)

project(ddb_gui_qt5 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set (CMAKE_SHARED_LIBRARY_PREFIX "")

find_package(Qt5 COMPONENTS Widgets Concurrent Svg Quick QuickWidgets QuickControls2 REQUIRED)

set(SOURCES
    widgets/CoverArt.cpp
    widgets/LogViewer.cpp
    widgets/Medialib.cpp
    widgets/PlaybackButtons.cpp
    widgets/Playlist.cpp
    widgets/PlaylistBrowser.cpp
    widgets/SeekSlider.cpp
    widgets/StatusBar.cpp
    widgets/TabBar.cpp
    widgets/VolumeSlider.cpp
    widgets/QueueManager.cpp
    AboutDialog.cpp
    ActionManager.cpp
    CoverArtCache.cpp
    CoverArtBackend.cpp
    DBApi.cpp
    DBFileDialog.cpp
    DeadbeefTranslator.cpp
    DefaultActions.cpp
    DefaultPlugins.cpp
    MainWindow.cpp
    MediasourceModel.cpp
    PlayItemModel.cpp
    PlayqueueModel.cpp
    PlaylistBrowserModel.cpp
    PlaylistModel.cpp
    PlaylistView.cpp
    PluginLoader.cpp
    QtGui.cpp
    QtGuiSettings.cpp
    SystemTrayIcon.cpp)
qt5_add_resources(SOURCES Resources.qrc)

add_library(ddb_gui_qt5 ${SOURCES})

target_link_libraries(ddb_gui_qt5 Qt5::Widgets Qt5::QuickWidgets Qt5::Concurrent)
target_include_directories(ddb_gui_qt5 PRIVATE
    .
    include)
