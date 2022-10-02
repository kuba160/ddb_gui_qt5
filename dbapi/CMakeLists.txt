cmake_minimum_required(VERSION 3.1.0)

project(dbapi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_FLAGS_INIT "-gdwarf-4")
#set(CMAKE_CXX_FLAGS_INIT "-fsanitize=undefined")

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set (CMAKE_SHARED_LIBRARY_PREFIX "")

message(STATUS "SET_QT_VERSION=${SET_QT_VERSION}")
if (SET_QT_VERSION STREQUAL Qt6)
    find_package(Qt6 COMPONENTS Core Concurrent Charts)
    if (USE_COVERART_CACHE)
        find_package(Qt6 COMPONENTS Gui)
    endif()
else()
    find_package(Qt5 COMPONENTS Core Concurrent Charts)
    if (USE_COVERART_CACHE)
        find_package(Qt5 COMPONENTS Gui)
    endif()
endif()

set(SOURCES
    Actions.cpp
    CoverArt.cpp
    Equalizer.cpp
    PlaybackControl.cpp
    PlaylistManager.cpp
    Settings.cpp
    Visualizations.cpp
    CoverArtBackend.cpp
    CoverArtCache.cpp
    DBApi.cpp
    models/MediasourceModel.cpp
    models/PlayItemModel.cpp
    models/PlaylistModel.cpp
    models/PlayqueueModel.cpp
    models/PlaylistBrowserModel.cpp
    models/PluginModel.cpp
    models/DRoleMapProxyModel.h models/DRoleMapProxyModel.cpp
    models/ActionsModel.h models/ActionsModel.cpp
    models/PlayItemTableProxyModel.h models/PlayItemTableProxyModel.cpp)
    #models/PlayItemSelectionModel.h models/PlayItemSelectionModel.cpp)

add_library(dbapi STATIC ${SOURCES})

target_link_libraries(dbapi Qt::Core Qt::Concurrent)

if (USE_COVERART_CACHE)
    target_link_libraries(dbapi Qt::Gui)
endif()

target_include_directories(dbapi PRIVATE
    include)
target_compile_definitions(dbapi PRIVATE USE_COVERART_CACHE=${USE_COVERART_CACHE} QT_USE_STRINGBUILDER)
