qt6_add_library(DDB2 STATIC)

set_source_files_properties(DDB2Globals.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(DDB2
    URI DeaDBeeF.Q.DDB2
    VERSION 1.0
    RESOURCE_PREFIX
        /qt/qml
    QML_FILES
        main.qml
        InfoQuick.qml
        SeekSlider.qml
        DDB2PlayItemViewDelegate.qml
        MainDrawer.qml
        PlaylistBrowser.qml
        SearchPage.qml
        RepeatShuffleButton.qml
        AboutPane.qml
        EqualizerPage.qml
        VolumeButton.qml
        QML_FILES ActionMenu.qml
        QML_FILES ActionMenuMain.qml
        QML_FILES DDB2Globals.qml
        QML_FILES Medialib.qml
        QML_FILES QueuePage.qml
        QML_FILES AboutQt.qml
        QML_FILES DDB2Page.qml
        QML_FILES SwipeViewTab.qml
        QML_FILES DDB2TextView.qml
        QML_FILES LogoPainter.qml
        QML_FILES VisPage.qml
        QML_FILES MedialibPresetPage.qml
)

