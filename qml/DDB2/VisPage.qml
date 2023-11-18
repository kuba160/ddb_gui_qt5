import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DDB2Page {
    title: "Visualizations"

   Component {
       id: osc
       Oscilloscope {
           property string instanceName: "DDB2"
      }
   }

   Component {
        id: osc_polar
        OscilloscopePolar {
            property string instanceName: "DDB2_POLAR"
       }
   }

   SwipeViewTab {
       anchors.fill: parent
       titles: ["Oscilloscope", "Polar Oscilloscope"]
       objs: [osc, osc_polar]
   }
}
