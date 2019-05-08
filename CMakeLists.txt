cmake_minimum_required (VERSION 2.6)

project (deadbeef-qt)

add_definitions(-DDEADBEEF_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")
if(NOT DEADBEEF_PREFIX)
    set(DEADBEEF_PREFIX "/usr/local")
endif(NOT DEADBEEF_PREFIX)

set(PREFIX ${DEADBEEF_PREFIX})

find_file(DEADBEEF_API deadbeef/deadbeef.h)

if(NOT DEADBEEF_API)
    message(FATAL_ERROR "Deadbeef api - not found")
else(NOT DEADBEEF_API)
    message(STATUS "Deadbeef api - found (${DEADBEEF_API})")
endif(NOT DEADBEEF_API)

include_directories(BEFORE SYSTEM ${CMAKE_INSTALL_PREFIX}/include)

add_definitions(-DARTWORK_ENABLED=1 -DHOTKEYS_ENABLED=1)

find_package(Qt5Widgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    include/callbacks.cpp
    include/parser.cpp
    include/qticonloader.cpp
    
    preferencesWidgets/InterfacePreferencesWidget.cpp
    preferencesWidgets/NetworkPreferencesWidget.cpp
    preferencesWidgets/PluginsPreferencesWidget.cpp
    preferencesWidgets/PluginSettingsWidget.cpp
    preferencesWidgets/SoundPreferencesWidget.cpp

    plugins/CoverArt/CoverArtWidget.cpp
    plugins/CoverArt/CoverArtWrapper.cpp
    plugins/CoverArt/CoverArtCache.cpp

    AboutDialog.cpp
    DBApiWrapper.cpp
    DBFileDialog.cpp
    GuiUpdater.cpp
    MainWindow.cpp
    PlayList.cpp
    PlayListModel.cpp
    PlayListWidget.cpp
    PluginLoader.cpp
    
    PreferencesDialog.cpp
    QtGui.cpp
    QtGuiSettings.cpp
    SeekSlider.cpp
    SystemTrayIcon.cpp
    TabBar.cpp
    VolumeSlider.cpp
    )

set(QT_HEADERS
    preferencesWidgets/InterfacePreferencesWidget.h
    preferencesWidgets/NetworkPreferencesWidget.h
    preferencesWidgets/PluginsPreferencesWidget.h
    preferencesWidgets/PluginSettingsWidget.h
    preferencesWidgets/SoundPreferencesWidget.h
    
    plugins/CoverArt/CoverArtWrapper.h
    plugins/CoverArt/CoverArtWidget.h
    plugins/CoverArt/CoverArtCache.h

    AboutDialog.h
    DBApiWrapper.h
    DBFileDialog.h
    GuiUpdater.h
    MainWindow.h
    PlayList.h
    PlayListModel.h
    PlayListWidget.h
    PluginLoader.h
    PreferencesDialog.h
    QtGuiSettings.h
    SeekSlider.h
    SystemTrayIcon.h
    TabBar.h
    VolumeSlider.h
    )

set(HEADERS
    include/callbacks.h
    include/parser.h
    include/qticonloader.h
    
    QtGui.h
    )

set(FORMS
    preferencesWidgets/InterfacePreferencesWidget.ui
    preferencesWidgets/NetworkPreferencesWidget.ui
    preferencesWidgets/PluginsPreferencesWidget.ui
    preferencesWidgets/SoundPreferencesWidget.ui
    
    AboutDialog.ui
    MainWindow.ui
    )

set(RESOURCES
    Resources.qrc
    )
     
include(${CMAKE_SOURCE_DIR}/plugins/CoverArt/CoverArt.cmake)
include(${CMAKE_SOURCE_DIR}/plugins/Hotkeys/Hotkeys.cmake)

qt5_wrap_cpp(MOC_SRCS ${QT_HEADERS})
qt5_wrap_ui(UIS_H ${FORMS})
qt5_add_resources(RCC ${RESOURCES})

add_library(ddb_gui_qt5 SHARED ${SOURCES} ${HEADERS} ${MOC_SRCS} ${UIS_H} ${RCC})

set_target_properties(ddb_gui_qt5 PROPERTIES PREFIX "")

target_link_libraries(ddb_gui_qt5 ${QT_LIBRARIES})
qt5_use_modules (ddb_gui_qt5 Widgets Concurrent)

install(TARGETS ddb_gui_qt5 LIBRARY DESTINATION lib/deadbeef/)
