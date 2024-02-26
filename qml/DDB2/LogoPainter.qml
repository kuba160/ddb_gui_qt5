import QtQuick 2.15
//import QtQuick.Controls 2.12
import QtQuick.Layouts 1.2
import QtQuick.Window 2.15
import QtQuick.Controls.Material 2.12

import DeaDBeeF.Q.DBApi 1.0
import "."


Item {
    id: qt_pane
    // Qt User Interface
    //anchors.fill: parent
    property int x_offset: width > height ? (width-height)/2 : 0
    property int y_offset: height > width ? (height-width)/2 : 0
    Layout.fillHeight: true
    Layout.fillWidth: true
    Canvas {
        id: canvas
        anchors.fill: parent

        property real lastX
        property real lastY
        property int distance: 0
        onPaint: {
            var ctx = getContext('2d')
            ctx.lineWidth = 1 * Screen.pixelDensity * (Math.min(height,width)/1000)
            let px = canvas2.ctx.getImageData(lastX, lastY, 1, 1).data;
            ctx.strokeStyle =  Qt.rgba(px[0]/255, px[1]/255, px[2]/255, px[3]/255)
            if (px[3] === 0) {
                ctx.globalCompositeOperation = "destination-out";
                ctx.strokeStyle =  Qt.rgba(px[0]/255, px[1]/255, px[2]/255, 255)

            }
            else {
                ctx.globalCompositeOperation = "source-over";
            }
            ctx.beginPath()
            ctx.moveTo(lastX, lastY)
            var dx = area.mouseX - lastX
            var dy = area.mouseY - lastY
            distance += Math.sqrt(dx * dx + dy * dy)
            lastX = area.mouseX
            lastY = area.mouseY
            ctx.lineTo(lastX, lastY)
            ctx.stroke()
        }
        Label {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 4
            text: canvas.distance
            opacity: 0.01
        }
        MouseArea {
            id: area
            preventStealing: true
            anchors.fill: parent
            onPressed: {
                canvas.lastX = mouseX
                canvas.lastY = mouseY
                canvas2.requestPaint()
            }
            onPositionChanged: {
                canvas.requestPaint()
            }
        }
    }
    Canvas {
        id:canvas2
        opacity: 0
        property var ctx
        anchors.fill: parent
        height: width
        onPaint: {
            ctx = getContext('2d')
            ctx.drawImage("/images/scalable.svg", x_offset, y_offset,
                          Math.min(width,height),Math.min(width,height))
        }
        Component.onCompleted: {
            loadImage("/images/scalable.svg")
        }
    }
}



/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
