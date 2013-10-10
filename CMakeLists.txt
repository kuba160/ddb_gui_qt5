cmake_minimum_required (VERSION 2.6)

project (deadbeef-qt)

add_definitions(-DDEADBEEF_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")
if(NOT DEADBEEF_PREFIX)
    set(DEADBEEF_PREFIX "/usr")
endif(NOT DEADBEEF_PREFIX)

set(PREFIX ${DEADBEEF_PREFIX})

find_file(DEADBEEF_API deadbeef/deadbeef.h)

if(NOT DEADBEEF_API)
    message(FATAL_ERROR "Deadbeef api - not found")
else(NOT DEADBEEF_API)
    message(STATUS "Deadbeef api - found (${DEADBEEF_API})")
endif(NOT DEADBEEF_API)

include_directories(BEFORE SYSTEM ${CMAKE_INSTALL_PREFIX}/include)

find_package(Qt4 4.4 COMPONENTS QtCore QtGui REQUIRED)

include(${QT_USE_FILE})

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
    
    AboutDialog.cpp
    DBApiWrapper.cpp
    DBFileDialog.cpp
    GuiUpdater.cpp
    MainWindow.cpp
    PlayList.cpp
    PlayListModel.cpp
    PlayListWidget.cpp
    
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
    
    AboutDialog.h
    DBApiWrapper.h
    DBFileDialog.h
    GuiUpdater.h
    MainWindow.h
    PlayList.h
    PlayListModel.h
    PlayListWidget.h
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
     
set(TRANSLATIONS
    QtGui_ru_RU.ts
    )

include(${CMAKE_SOURCE_DIR}/plugins/CoverArt/CoverArt.cmake)
include(${CMAKE_SOURCE_DIR}/plugins/Hotkeys/Hotkeys.cmake)
configure_file(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_SOURCE_DIR}/config.h)

qt4_wrap_cpp(MOC_SRCS ${QT_HEADERS})
qt4_wrap_ui(UIS_H ${FORMS})
qt4_add_resources(RCC ${RESOURCES})
qt4_add_translation(COMPILED_TRANSLATIONS ${TRANSLATIONS})

add_library(ddb_gui_Qt SHARED ${SOURCES} ${QT_HEADERS} ${HEADERS} ${MOC_SRCS} ${UIS_H} ${RCC} ${COMPILED_TRANSLATIONS})

set_target_properties(ddb_gui_Qt PROPERTIES PREFIX "")

target_link_libraries(ddb_gui_Qt ${QT_LIBRARIES})

install(TARGETS ddb_gui_Qt LIBRARY DESTINATION lib/deadbeef/)
install(FILES ${COMPILED_TRANSLATIONS} DESTINATION share/deadbeef/translations)
