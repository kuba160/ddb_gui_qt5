option(HOTKEYS_ENABLED "Global hotkeys support" ON)

if (HOTKEYS_ENABLED)
    find_file(DEADBEEF_HOTKEYS hotkeys.so PATHS ${DEADBEEF_PREFIX}/lib/deadbeef)
    if (DEADBEEF_HOTKEYS)
        message(STATUS "Deadbeef Global Hotkeys plugin - found ${DEADBEEF_HOTKEYS}")
        set(${HOTKEYS_ENABLED} ON)
    else (DEADBEEF_HOTKEYS)
        message(STATUS "Deadbeef Global Hotkeys plugin - not found")
        set(${HOTKEYS_ENABLED} OFF)
    endif(DEADBEEF_HOTKEYS)
endif(HOTKEYS_ENABLED)

if (HOTKEYS_ENABLED)
    set(HEADERS ${HEADERS}
        plugins/Hotkeys/include/hotkeys.h
    )
    set(QT_HEADERS ${QT_HEADERS}
        plugins/Hotkeys/HotkeysTreeWidget.h
        plugins/Hotkeys/HotkeyReadDialog.h
        plugins/Hotkeys/HotkeysWidget.h
        )
    set(SOURCES ${SOURCES}
        plugins/Hotkeys/HotkeysTreeWidget.cpp
        plugins/Hotkeys/HotkeyReadDialog.cpp
        plugins/Hotkeys/HotkeysWidget.cpp
        )
    set(TRANSLATIONS ${TRANSLATIONS}
        plugins/Hotkeys/HotkeysPlugin_ru_RU.ts
        )
endif (HOTKEYS_ENABLED)