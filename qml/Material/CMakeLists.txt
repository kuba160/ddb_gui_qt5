qt6_add_library(Material STATIC)
qt_add_qml_module(Material
    URI DeaDBeeF.Q.Material
    VERSION 1.0
    RESOURCE_PREFIX
        /qt/qml
    QML_FILES
        ActionsTree.qml
        EqualizerQuick.qml
        InfoQuick.qml
        OscilloscopePolar.qml
        OscilloscopePolar_qt6.qml
        Oscilloscope.qml
        Oscilloscope_qt6.qml
        PlaybackButtons.qml
        PlaylistBrowserQuick.qml
        PluginBrowser.qml
        QueueManagerQuick.qml
        SeekSliderDeadbeef.qml
        SeekSlider.qml
        TabBarQuick.qml
        VolumeSliderQuickDeadbeef.qml
        VolumeSliderQuick.qml
        WidgetBrowser.qml
        main.qml
)
