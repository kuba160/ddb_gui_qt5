#ifndef DBFILEDIALOG_H
#define DBFILEDIALOG_H

#include <QFileDialog>


class DBFileDialog : public QFileDialog {
    Q_OBJECT
public:

    enum DialogType {
        OPEN_FILES = 0,
        ADD_FILES,
        ADD_FOLDERS,
        LOAD_PLAYLIST,
        SAVE_PLAYLIST
    };

    DBFileDialog(QWidget *parent, DialogType type);

private:
    DialogType m_type;

public slots:
    void onAccepted();
    //void on_actionAddFolder_triggered();
};

#endif // DBFILEDIALOG_H
