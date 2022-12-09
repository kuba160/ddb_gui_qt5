cmake_minimum_required(VERSION 3.1.0)


if (NOT DEFINED SET_QT_VERSION)
    set(SET_QT_VERSION Qt6)
endif()
set (USE_WIDGETS 1)
set(CMAKE_CXX_FLAGS_INIT "-gdwarf-4")
#set(CMAKE_CXX_FLAGS_INIT "-fsanitize=undefined")

set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)
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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 COMPONENTS REQUIRED Core Concurrent Gui Quick Svg Charts)
if (USE_WIDGETS)
    find_package(Qt6 COMPONENTS REQUIRED Widgets QuickWidgets)
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
add_library(ddb_gui_qt5 SHARED ${SOURCES})


# fix for Qt Creator to find qml files
list(APPEND QML_DIRS "${CMAKE_BINARY_DIR}/qml")
set(QML_IMPORT_PATH "${QML_DIRS}" CACHE STRING "Qt Creator 4.1 extra qml import paths")

# Add qml modules
add_subdirectory(qml/GuiCommon)
add_subdirectory(qml/DDB2)
# add_subdirectory(qml/Material)



target_link_libraries(ddb_gui_qt5 dbapiplugin DDB2 GuiCommon DDB2plugin GuiCommonplugin Qt6::Core Qt::Concurrent Qt::Gui Qt::Quick)
if (USE_WIDGETS)
    target_link_libraries(ddb_gui_qt5 dbapi Qt6::Widgets Qt6::QuickWidgets)
endif()

target_include_directories(ddb_gui_qt5 PRIVATE ".")

target_compile_definitions(ddb_gui_qt5 PRIVATE USE_WIDGETS=${USE_WIDGETS} QT_USE_STRINGBUILDER)
target_compile_definitions(ddb_gui_qt5 PRIVATE USE_
    _CACHE=${USE_COVERART_CACHE})

target_compile_options(ddb_gui_qt5 PRIVATE "-gdwarf-4")

install(TARGETS ddb_gui_qt5 DESTINATION "~/.local/lib/deadbeef")
