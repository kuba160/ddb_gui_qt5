import QtQuick 2.15

// select file/folder dialog
import Qt.labs.platform
//import QtQuick.Dialogs


import DeaDBeeF.Q.DBApi 1.0
import "."

QtObject {
    id: mainDialog

    property var fd: FolderDialog {
         id: folderDialog

    //     //currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
         //selectedFolder: viewer.folder

    }


    function bindActions() {
        //DBApi.actions.getAction("q_open_files").actionApplied.connect(folderDialog.open)
        //DBApi.actions.getAction("q_add_files").actionApplied.connect(folderDialog.open)
        DBApi.actions.getAction("q_add_folders").actionApplied.connect(folderDialog.open)
        //DBApi.actions.getAction("q_add_location").actionApplied.connect(folderDialog.open)
        DBApi.actions.getAction("q_load_playlist").actionApplied.connect(folderDialog.open)
        DBApi.actions.getAction("q_save_playlist").actionApplied.connect(folderDialog.open)
    }
}
/*##^##
Designer {
D{i:0;formeditorZoom:4;height:44;width:86}
}
##^##*/
