#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QToolBar>
#include <QBoxLayout>

#include "SystemTrayIcon.h"
#include "VolumeSlider.h"
#include "SeekSlider.h"
#include "PlayList.h"

#include <plugins/CoverArt/CoverArtWidget.h>

namespace Ui {
class MainWindow;
}

enum ActionOnClose {
    Exit = 0,
    Hide = 1,
    Minimize = 2,
};

class MainWindow : public QMainWindow, DBToolbarWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr, DBApi *Api = nullptr);
    ~MainWindow();

    DBApi *Api();

protected:
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);

    void loadIcons();

private:
    
    void loadConfig();
    void saveConfig();
    
    ActionOnClose actionOnClose;
    void configureActionOnClose(bool minimizeOnClose, bool hideTrayIcon);

    Ui::MainWindow *ui;

public:
//    DBApi *api;

private:
    SystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    VolumeSlider volumeSlider;
    SeekSlider progressBar;
    PlayList playList;


    QToolBar *ToolbarStack[64];
    char ToolbarStackCount;

    CoverArtWidget coverArtWidget;

    QActionGroup orderGroup;
    QActionGroup loopingGroup;

    void createConnections();

    void createToolBars();
    void createTray();
    QMenu *createPopupMenu();

    void updateTitle(DB_playItem_t *it = nullptr);
    
    void loadActions();

public slots:
    void on_actionAddURL_triggered();
    void on_actionAddAudioCD_triggered();
    void on_actionAddFiles_triggered();
    void on_actionPreferences_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionLoopNothing_triggered();
    void on_actionLoopTrack_triggered();
    void on_actionLoopAll_triggered();
    void on_actionShuffleOrder_triggered();
    void on_actionRandomOrder_triggered();
    void on_actionLinearOrder_triggered();
    void on_actionExit_triggered();
    void on_actionPause_triggered();
    void on_actionPrev_triggered();
    void on_actionNext_triggered();
    void on_actionStop_triggered();
    void on_actionPlay_triggered();
    void on_actionAddFolder_triggered();
    void on_actionClearAll_triggered();
    void on_actionSelectAll_triggered();
    void on_actionDeselectAll_triggered();
    void on_actionRemove_triggered();

    void on_actionSaveAsPlaylist_triggered();
    void on_actionLoadPlaylist_triggered();

    void on_actionHideMenuBar_triggered();
    void on_actionBlockToolbarChanges_triggered();

    // TODO: These were disabled, so now there is no way to enable artwork back :D
    //void on_actionHideCoverArt_triggered();
    // void onCoverartClose();

    void trackChanged(DB_playItem_t *, DB_playItem_t *);

    void setCloseOnMinimized(bool);
    void setTrayIconHidden(bool);
    void titleSettingChanged();
    void on_actionPlayListHeader_triggered();
    void on_actionHideTabBar_triggered();
    
    void on_deadbeefActivated();

    void windowActivate();
    void windowShowHide();

    void windowAddToolbar(QToolBar *);
};

extern MainWindow *w;

#endif // MAINWINDOW_H
