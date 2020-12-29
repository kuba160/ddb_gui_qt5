#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QToolBar>
#include <QBoxLayout>

#include "SystemTrayIcon.h"
#include "DBApi.h"



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
    QMenuBar *mainMenu;
    void loadConfig();
    void saveConfig();
    
    ActionOnClose actionOnClose;
    void configureActionOnClose(bool minimizeOnClose, bool hideTrayIcon);

    SystemTrayIcon *trayIcon;
    QMenu *trayMenu;

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
    void on_actionAddFolder_triggered();

    void on_actionSaveAsPlaylist_triggered();
    void on_actionLoadPlaylist_triggered();

    void trackChanged(DB_playItem_t *, DB_playItem_t *);

    void setCloseOnMinimized(bool);
    void setTrayIconHidden(bool);
    void titleSettingChanged();
    
    void on_deadbeefActivated();

    void windowActivate();
    void windowShowHide();

    void windowAddToolbar(QToolBar *);
    void windowAddDockable(QDockWidget *);

signals:
    void configLoaded();
};

extern MainWindow *w;

#endif // MAINWINDOW_H
