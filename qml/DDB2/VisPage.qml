import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.2
import QtQuick.Controls.Material 2.15

import DeaDBeeF.Q.DBApi 1.0
import DeaDBeeF.Q.GuiCommon 1.0

DDB2Page {
    title: "Visualizations"

    Component {
        id: osc_shader
        OscilloscopeShader {
            property string instanceNAME: "ScopeShader"
        }
    }

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
       titles: ["Oscilloscope Shader", "Oscilloscope", "Polar Oscilloscope"]
       objs: [osc_shader, osc, osc_polar]
   }
}
