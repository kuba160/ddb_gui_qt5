cmake_minimum_required(VERSION 3.1.0)


if (NOT DEFINED SET_QT_VERSION)
    set(SET_QT_VERSION Qt5)
endif()
set (USE_WIDGETS 0)
set(CMAKE_CXX_FLAGS_INIT "-gdwarf-4")
#set(CMAKE_CXX_FLAGS_INIT "-fsanitize=undefined")

add_subdirectory(dbapi)


project(ddb_gui_q LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set (CMAKE_SHARED_LIBRARY_PREFIX "")

message(STATUS "SET_QT_VERSION=${SET_QT_VERSION}")
if (SET_QT_VERSION STREQUAL Qt6)
    find_package(Qt6 COMPONENTS Core Concurrent Gui Quick Svg Charts)
    if (USE_WIDGETS)
        find_package(Qt6 COMPONENTS Widgets QuickWidgets)
    endif()
else()
    find_package(Qt5 COMPONENTS Core Concurrent Gui Quick Svg Charts)
    if (USE_WIDGETS)
        find_package(Qt5 COMPONENTS Widgets QuickWidgets)
    endif()
endif()


if (USE_WIDGETS)
    set (SOURCES_WIDGETS
        widgets/plugins/PlaybackButtons.cpp
        widgets/plugins/SeekSlider.cpp
        widgets/plugins/VolumeSlider.cpp
        widgets/plugins/StatusBar.cpp
        widgets/plugins/TabBar.cpp
        widgets/CoverArtWidgetCache.cpp
        widgets/DefaultPlugins.cpp
        #widgets/PlaylistView.cpp
        widgets/PluginWidgetsLoader.cpp
        widgets/PluginManager.cpp
        widgets/MainWindow.cpp
        widgets/PlaylistView.cpp
        widgets/plugins/Playlist.h widgets/plugins/Playlist.cpp
        widgets/plugins/ActionsTree.h widgets/plugins/ActionsTree.cpp
        widgets/plugins/QueueManager.cpp widgets/plugins/QueueManager.h
        widgets/DBActionMenu.h widgets/DBActionMenu.cpp
)
endif()

set(SOURCES
    ${SOURCES_WIDGETS}
    PluginLoader.cpp
    WidgetLibraryModel.cpp
    QtGui.cpp)
qt_add_resources(SOURCES Images.qrc)
qt_add_resources(SOURCES Qml.qrc)


add_library(ddb_gui_qt5 SHARED ${SOURCES})

if (SET_QT_VERSION STREQUAL Qt6)
    target_link_libraries(ddb_gui_qt5 dbapi Qt6::Core Qt::Concurrent Qt::Gui Qt::Quick)
    if (USE_WIDGETS)
        target_link_libraries(ddb_gui_qt5 dbapi Qt6::Widgets Qt6::QuickWidgets)
    endif()
else()
    target_link_libraries(ddb_gui_qt5 dbapi Qt5::Core Qt5::Concurrent Qt5::Gui Qt5::Quick)
    if (USE_WIDGETS)
        target_link_libraries(ddb_gui_qt5 dbapi Qt5::Widgets Qt5::QuickWidgets)
    endif()
endif()

target_include_directories(ddb_gui_qt5 PRIVATE ".")

target_compile_definitions(ddb_gui_qt5 PRIVATE USE_WIDGETS=${USE_WIDGETS} QT_USE_STRINGBUILDER)
target_compile_definitions(ddb_gui_qt5 PRIVATE USE_COVERART_CACHE=${USE_COVERART_CACHE})

target_compile_options(ddb_gui_qt5 PRIVATE "-gdwarf-4")

install(TARGETS ddb_gui_qt5 DESTINATION "~/.local/lib/deadbeef")
