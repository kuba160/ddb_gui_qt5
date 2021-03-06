#ifndef DEFAULTACTIONS_H
#define DEFAULTACTIONS_H

#include <QWidget>
#include <QMenuBar>
#include <QDialog>
#include <QFormLayout>
#include "DBApi.h"
#include "PlaylistView.h"
//#include "ui_DefaultActions.h"

namespace Ui {
class DefaultActions;
}

class DefaultActions : public QWidget, public DBWidget
{
    Q_OBJECT
public:
    explicit DefaultActions(DBApi *Api = nullptr, QWidget *parent = nullptr);
    QMenuBar *getDefaultMenuBar();
private:
    Ui::DefaultActions *ui;

    QActionGroup *repeatGroup;
    QAction *repeat[3];
    QActionGroup *shuffleGroup;
    QAction *shuffle[4];

    QMenu *main_widgets;
    QActionGroup *main_widgets_list;
    QMenu *new_plugins;
    QMenu *remove_plugins;

    PlaylistView *pv_search = nullptr;

    void sortPlaylist(const char *format, bool ascending);
public slots:
    void shuffleRepeatHandler();
    // on triggered menu actions
    void onWidgetAdd();
    void onWidgetRemove();
    // on triggered main widget
    void onWidgetMain();
    //
    void onWidgetToggle(bool toggle);
    // on new/removed widget
    void onWidgetAdded(int num);
    void onWidgetRemoved(QString internalName);
    void onWidgetLibraryAdded(DBWidgetInfo i);

    void onTrackChanged();

private slots:
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionPreferences_triggered();
    void on_actionExit_triggered();
    void on_actionScrollPlayback_triggered(bool checked);
    void on_actionCursorPlayback_triggered(bool checked);
    void on_actionStopTrack_triggered(bool checked);
    void on_actionStopAlbum_triggered(bool checked);
    void on_actionJump_to_current_track_triggered();
    void on_actionOpenFiles_triggered();
    void on_actionNewPlaylist_triggered();
    void on_actionLoadPlaylist_triggered();
    void on_actionSaveAsPlaylist_triggered();
    void on_actionAddFolder_triggered();
    void on_actionAddFiles_triggered();
    void on_actionAddAudioCD_triggered();
    void on_actionAddURL_triggered();
    void on_actionClearAll_triggered();
    void on_actionSelectAll_triggered();
    void on_actionDeselectAll_triggered();
    void on_actionInvert_selection_triggered();
    void on_actionSelectionRemove_triggered();
    void on_actionSelectionCrop_triggered();
    void on_actionSortTitle_triggered();
    void on_actionSortTrackNumber_triggered();
    void on_actionSortArtist_triggered();
    void on_actionSortAlbum_triggered();
    void on_actionSortDate_triggered();
    void on_actionSortRandom_triggered();
    void on_actionSortCustom_triggered();
    void on_actionFind_triggered();
    void actionFind_searchBox_edited(const QString);
};

#endif // DEFAULTACTIONS_H
