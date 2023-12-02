import QtQuick 2.15
//import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import "."

DDB2Page {
    id: page
    title: "About DeaDBeeF"

    property int defaultPane: 0


    page_menu: Menu {
        id: extra_menu
        property list<string> extra_names: ["ChangeLog", "GPLv2", "LGPLv2.1"]
        property list<string> extra_text: [DBApi.conf.aboutChangelog, DBApi.conf.aboutGPLV2,
                                 DBApi.conf.aboutLGPLV21]

        Repeater {
            model: extra_menu.extra_names.length
            MenuItem {
                text: extra_menu.extra_names[modelData]

                Component {
                    id: text_page
                    DDB2Page {
                        title: extra_menu.extra_names[modelData]
                        DDB2TextView {
                            text: extra_menu.extra_text[modelData]
                        }
                    }
                }
                onClicked: {
                    DDB2Globals.stack.push(text_page)
                }
            }
        }

        MenuItem {
            text: "About Qt"

            onClicked: {
                about_dialog.open()
            }
        }
    }

    AboutQt {id: about_dialog}

    Component {
        id: info_deadbeef
        DDB2TextView {
            text: DBApi.conf.aboutText
        }
    }

    Component {
        id: info_q
        LogoPainter {}
    }

    Component {
        id: info_translators
        DDB2TextView {
            text: DBApi.conf.aboutTranslators
        }
    }

    SwipeViewTab {
        titles: ["DeaDBeeF", "Q User Interface", "Translators"]
        objs: [info_deadbeef, info_q, info_translators]
        index_default: defaultPane
        anchors.fill: parent
    }
}


/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
