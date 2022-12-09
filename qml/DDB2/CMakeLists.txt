qt6_add_library(DDB2 STATIC)
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
        SearchPane.qml
        RepeatShuffleButton.qml
        AboutPane.qml
        EqualizerPane.qml
)
