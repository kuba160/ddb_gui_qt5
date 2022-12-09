
qt6_add_library(GuiCommon STATIC)
qt_add_qml_module(GuiCommon
    URI DeaDBeeF.Q.GuiCommon
    VERSION 1.0
    RESOURCE_PREFIX
        /qt/qml
    QML_FILES
        CoverArt.qml
        DBWidget.qml
        MainPane.qml
        PlayItemViewDelegate.qml
        PlayItemView.qml
        PlaylistQuick.qml
)
