#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QToolBar>
#include <QBoxLayout>

#include "SystemTrayIcon.h"
#include "DBApi.h"


namespace Ui {
class MainWindow;
}

enum ActionOnClose {
    Exit = 0,
    Hide = 1,
    Minimize = 2,
};

class MainWindow : public QMainWindow, DBWidget {
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
    SystemTrayIcon *trayIcon;
    QMenu *trayMenu;

    QMenu *main_widgets;
    QActionGroup *main_widgets_list;
    QMenu *new_plugins;
    QMenu *remove_plugins;

    QActionGroup *repeatGroup;
    QAction *repeat[3];
    QActionGroup *shuffleGroup;
    QAction *shuffle[4];

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
    void on_actionExit_triggered();
    void on_actionAddFolder_triggered();

    void on_actionSaveAsPlaylist_triggered();
    void on_actionLoadPlaylist_triggered();

    // TODO: These were disabled, so now there is no way to enable artwork back :D
    //void on_actionHideCoverArt_triggered();
    // void onCoverartClose();

    void trackChanged(DB_playItem_t *, DB_playItem_t *);

    void setCloseOnMinimized(bool);
    void setTrayIconHidden(bool);
    void titleSettingChanged();
    
    void on_deadbeefActivated();

    void windowActivate();
    void windowShowHide();


    void windowAddToolbar(QToolBar *);
    void windowAddDockable(QDockWidget *);
    void windowViewActionAdd(QAction *);
    void windowViewActionCreate(QAction *);
    void windowViewActionRemove(QAction *);
    void windowViewActionRemoveToggleHide(bool visible);
    void windowViewActionMainWidget(QAction *);

    void shuffleRepeatHandler();

signals:
    void configLoaded();
};

extern MainWindow *w;

#endif // MAINWINDOW_H
