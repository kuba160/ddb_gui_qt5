import QtQuick 2.15
//import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2

import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import "."

Dialog {
    id: mainDialog
    anchors.centerIn: parent
    margins: 16
    //padding: 16
    width: parent.width*0.9
    //height: Math.min(parent.height*0.9, implicitHeight)
    ColumnLayout {
        id: col_layout
        anchors.fill: parent
//        Image {
//            source: ":/qt-project.org/qmessagebox/images/qtlogo-64.png"
//            width: 64
//            height: 64
//        }
        Row {
            Label {
                text: "About Qt"
                font.pixelSize: 16
            }
        }
        ScrollView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            contentWidth: availableWidth
            contentHeight: implicitContentHeight

            TextArea {
                background: Item{}
                id: textBox
                textFormat: TextEdit.RichText
                leftPadding: 0


                text:     ("<p>This program uses Qt Version %4.</p>" +
                          "<p>Qt is a C++ toolkit for cross-platform application " +
                          "development.</p>" +
                          "<p>Qt provides single-source portability across all major desktop " +
                          "operating systems. It is also available for embedded Linux and other " +
                          "embedded and mobile operating systems.</p>" +
                          "<p>Qt is available under multiple licensing options designed " +
                          "to accommodate the needs of our various users.</p>" +
                          "<p>Qt licensed under our commercial license agreement is appropriate " +
                          "for development of proprietary/commercial software where you do not " +
                          "want to share any source code with third parties or otherwise cannot " +
                          "comply with the terms of GNU (L)GPL.</p>" +
                          "<p>Qt licensed under GNU (L)GPL is appropriate for the " +
                          "development of Qt&nbsp;applications provided you can comply with the terms " +
                          "and conditions of the respective licenses.</p>" +
                          "<p>Please see <a href=\"http://%2/\">%2</a> " +
                          "for an overview of Qt licensing.</p>" +
                          "<p>Copyright (C) %1 The Qt Company Ltd and other " +
                          "contributors.</p>" +
                          "<p>Qt and the Qt logo are trademarks of The Qt Company Ltd.</p>" +
                          "<p>Qt is The Qt Company Ltd product developed as an open source " +
                          "project. See <a href=\"http://%3/\">%3</a> for more information.</p>")
                           .arg("2023").arg("qt.io/licensing").arg("qt.io").arg(_qt_version)
                readOnly: true
                selectByMouse: false
                wrapMode: TextEdit.Wrap
                color: "white"

                onLinkActivated: (link) => { Qt.openUrlExternally(link) }
            }
        }
        Button {
            text: "Ok"
            Layout.alignment: Qt.AlignRight
            onClicked: {
                mainDialog.close()
            }
        }
    }
}


/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
