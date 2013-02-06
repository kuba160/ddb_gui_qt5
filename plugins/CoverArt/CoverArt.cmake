option(ARTWORK_ENABLED "Enable artwork support" ON)

if (ARTWORK_ENABLED)
    find_file(DEADBEEF_ARTWORK artwork.so PATHS ${DEADBEEF_PREFIX}/lib/deadbeef)
    if(DEADBEEF_ARTWORK)
        message(STATUS "Deadbeef Artwork plugin - found ${DEADBEEF_ARTWORK}")
        set(${ARTWORK_ENABLED} ON)
    else(DEADBEEF_ARTWORK)
        message(STATUS "Deadbeef Artwork plugin - not found")
        set(${ARTWORK_ENABLED} OFF)
    endif(DEADBEEF_ARTWORK)
endif (ARTWORK_ENABLED)

if (ARTWORK_ENABLED)
    set(SOURCES ${SOURCES}
        plugins/CoverArt/CoverArtWidget.cpp
        plugins/CoverArt/CoverArtWrapper.cpp
        plugins/CoverArt/CoverArtCache.cpp
        )
    set(QT_HEADERS ${QT_HEADERS}
        plugins/CoverArt/CoverArtWrapper.h
        plugins/CoverArt/CoverArtWidget.h
        plugins/CoverArt/CoverArtCache.h
        )
    set(HEADERS ${HEADERS}
        plugins/CoverArt/include/artwork.h
        )
    set(TRANSLATIONS ${TRANSLATIONS}
        plugins/CoverArt/CoverArtPlugin_ru_RU.ts
        )
endif (ARTWORK_ENABLED)