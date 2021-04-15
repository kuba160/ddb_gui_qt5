#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>
#include <QToolBar>
#include <QBoxLayout>
#include <QTimer>

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
    void closeEvent(QCloseEvent *);

private:
    QMenuBar *mainMenu;
    void loadConfig();
    void saveConfig();
    
    ActionOnClose actionOnClose;
    void configureActionOnClose(bool minimizeOnClose, bool hideTrayIcon);

    SystemTrayIcon *trayIcon = nullptr;
    QMenu *trayMenu = nullptr;

    void createTray();


    QTimer title_updater;

private slots:
    void updateTitle();

public slots:
    void trackChanged(DB_playItem_t *, DB_playItem_t *);

    void setCloseOnMinimized(bool);
    void setTrayIconHidden(bool);
    void titleSettingChanged();
    
    void on_deadbeefActivated();

    void windowActivate();
    void windowShowHide();


signals:
    void configLoaded();
};

extern MainWindow *w;

#endif // MAINWINDOW_H
