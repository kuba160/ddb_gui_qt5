#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>

#include "config.h"

#include "SystemTrayIcon.h"
#include "StatusBar.h"
#include "VolumeSlider.h"
#include "SeekSlider.h"

#ifdef ARTWORK_ENABLED
#include <plugins/CoverArt/CoverArtWidget.h>
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();


protected:
    void changeEvent(QEvent *);
    void closeEvent(QCloseEvent *);

    void loadIcons();

private:
    enum ActionOnClose {
        Exit = 0,
        Hide = 1,
        Minimize = 2,
    };
    
    void loadConfig();
    void saveConfig();
    
    ActionOnClose actionOnClose;
    void configureActionOnClose(bool minimizeOnClose, bool hideTrayIcon);

    Ui::MainWindow *ui;

    SystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    StatusBar *dbStatusBar;
    VolumeSlider volumeSlider;
    SeekSlider progressBar;

#ifdef ARTWORK_ENABLED
    CoverArtWidget coverArtWidget;
#endif

    QActionGroup orderGroup;
    QActionGroup loopingGroup;

    void createConnections();

    void createToolBars();
    void createTray();
    void createStatusbar();
    QMenu *createPopupMenu();

    void updateTitle(DB_playItem_t *it = NULL);

private slots:
    void on_actionAddURL_activated();
    void on_actionAddAudioCD_activated();
    void on_actionAddFiles_activated();
    void on_actionPreferences_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionLoopNothing_triggered();
    void on_actionLoopTrack_triggered();
    void on_actionLoopAll_triggered();
    void on_actionShuffleOrder_triggered();
    void on_actionRandomOrder_triggered();
    void on_actionLinearOrder_triggered();
    void on_actionExit_activated();
    void on_actionPause_activated();
    void on_actionPrev_activated();
    void on_actionNext_activated();
    void on_actionStop_activated();
    void on_actionPlay_activated();
    void on_actionAddFolder_activated();
    void on_actionClearAll_activated();
    void on_actionSelectAll_activated();
    void on_actionDeselectAll_activated();
    void on_actionRemove_activated();

    void on_actionSaveAsPlaylist_activated();
    void on_actionLoadPlaylist_activated();

    void on_actionHideMenuBar_activated();
    void on_actionHideStatusbar_activated();
    void on_actionBlockToolbarChanges_activated();

#ifdef ARTWORK_ENABLED
    void on_actionHideCoverArt_activated();
    void onCoverartClose();
#endif

    void trayIcon_wheeled(int);
    void trayIcon_activated(QSystemTrayIcon::ActivationReason);

    void trackChanged(DB_playItem_t *, DB_playItem_t *);

    void setCloseOnMinimized(bool);
    void setTrayIconHidden(bool);
    void titleSettingChanged();
    void on_actionPlayListHeader_triggered();
    void on_actionHideTabBar_triggered();
    void on_actionToggleHideMenu_triggered();
};

#endif // MAINWINDOW_H
