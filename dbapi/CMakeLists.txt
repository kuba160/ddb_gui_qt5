cmake_minimum_required(VERSION 3.1.0)

project(dbapi LANGUAGES C CXX)

set(USE_COVERART_CACHE 1)

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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "SET_QT_VERSION=${SET_QT_VERSION}")
if (SET_QT_VERSION STREQUAL Qt6)
    find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Charts Qml)
    if (USE_COVERART_CACHE)
        find_package(Qt6 COMPONENTS Gui)
    endif()
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Concurrent Charts Qml)
    if (USE_COVERART_CACHE)
        find_package(Qt5 COMPONENTS Gui)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Quick)

qt6_add_library(dbapi STATIC)

qt6_add_qml_module(dbapi
    URI DeaDBeeF.Q.DBApi
    VERSION 1.0
    RESOURCE_PREFIX
        /qt/qml
    SOURCES
        Actions.cpp Actions.h
        ActionsDefault.cpp
        ActionOwner.h ActionOwner.cpp
        actions/ActionsConfig.h actions/ActionsConfig.cpp
        actions/ActionsPlaceholder.h actions/ActionsPlaceholder.cpp
        CoverArt.cpp
        Equalizer.cpp
        ItemImporter.cpp ItemImporter.h
        PlaybackControl.cpp
        PlaylistManager.cpp PlaylistManager.h
        PlayItemIterator.cpp PlayItemIterator.h
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
        models/PlayItemFilterModel.h models/PlayItemFilterModel.cpp
        models/PlayItemTableProxyModel.h models/PlayItemTableProxyModel.cpp
        actions/ActionSpec.h actions/ActionSpec.cpp
        actions/ActionsDeadbeef.h actions/ActionsDeadbeef.cpp
        models/TreeNode.h models/TreeNode.cpp
        models/ScriptableModel.h models/ScriptableModel.cpp
        scriptable/scriptable.c scriptable/scriptable.h
        models/ScriptableSimple.h models/ScriptableSimple.cpp
        SOURCES MedialibConfig.h MedialibConfig.cpp
)

target_link_libraries(dbapi PRIVATE Qt::Core Qt::Concurrent)
target_include_directories(dbapi PRIVATE include)
target_compile_definitions(dbapi PRIVATE USE_COVERART_CACHE=${USE_COVERART_CACHE} QT_USE_STRINGBUILDER)
