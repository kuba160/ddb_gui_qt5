#include "MainWindow.h"

#include <QSettings>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QToolBar>

#include "QtGuiSettings.h"

#include "QtGui.h"
#include "DBApi.h"
#include "AboutDialog.h"

#include <QtConcurrent>
#include <QFutureWatcher>
#include "DBFileDialog.h"
#include "DeadbeefTranslator.h"

MainWindow::MainWindow(QWidget *parent, DBApi *Api) :
        QMainWindow(parent),
        DBWidget (parent, Api) {

    // MenuBar
    mainMenu = Api->getMainMenuBar();
    setMenuBar(mainMenu);
    mainMenu->setNativeMenuBar(true);

    setContextMenuPolicy(Qt::NoContextMenu);
    setAttribute(Qt::WA_DeleteOnClose);

    // Connections
    connect(api, SIGNAL(trackChanged(DB_playItem_t*,DB_playItem_t*)), this, SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    connect(api, SIGNAL(deadbeefActivated()), this, SLOT(on_deadbeefActivated()));


    setWindowIcon(QIcon(":/root/images/deadbeef.png"));

    connect(&title_updater, SIGNAL(timeout()), this, SLOT(updateTitle()));
    title_updater.start(1000);

    updateTitle();
    actionOnClose = Exit;
}

MainWindow::~MainWindow() {
    delete trayIcon;
}

DBApi* MainWindow::Api() {
    return api;
}


void MainWindow::createTray() {
    trayIcon = new SystemTrayIcon(this);
    trayMenu = new QMenu(this);
    // TODO
    /*
    trayMenu->addAction(ui->actionPlay);
    trayMenu->addAction(ui->actionPause);
    trayMenu->addAction(ui->actionStop);
    trayMenu->addAction(ui->actionNext);
    trayMenu->addAction(ui->actionPrev);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->actionExit);
    trayIcon->setContextMenu(trayMenu);
    */
    QIcon icon(":/root/images/deadbeef.png");
    trayIcon->setIcon(icon);

    //connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIcon_activated(QSystemTrayIcon::ActivationReason)));
    //connect(trayIcon, SIGNAL(wheeled(int)), this, SLOT(trayIcon_wheeled(int)));
    connect(trayIcon, SIGNAL(singleClick()), this, SLOT(windowActivate()));
    connect(trayIcon, SIGNAL(doubleClick()), this, SLOT(windowShowHide()));
    connect(trayIcon, SIGNAL(middleClick()), api, SLOT(togglePause()));
    //connect(trayIcon, SIGNAL(wheelScroll(int)), &volumeSlider, SLOT(adjustVolume(int)));

    trayIcon->setVisible(true);
}

void MainWindow::titleSettingChanged() {
    updateTitle();
}

void MainWindow::updateTitle() {
    DB_playItem_t* curr_track = DBAPI->streamer_get_playing_track ();
    ddb_playlist_t *plt;
    if (curr_track)
        plt = DBAPI->pl_get_playlist (curr_track);
    else
        plt = DBAPI->plt_get_curr ();
    ddb_tf_context_t context;
    context._size = sizeof(ddb_tf_context_t);
    context.flags = 0;
    context.it = curr_track;
    context.plt = plt;
    context.idx = 0;
    context.id = 0;
    context.iter = PL_MAIN;
    context.update = 0;
    context.dimmed = 0;

    // TODO: Make this customizable
    QString script = nullptr;
    if (!curr_track)
        script = SETTINGS->getValue("MainWindow", "TitlebarStopped", "DeaDBeeF %_deadbeef_version%").toString();
    else
        script = SETTINGS->getValue("MainWindow", "TitlebarPlaying", "%artist% - %title% - DeaDBeeF %_deadbeef_version%").toString();

    char * code_script = DBAPI->tf_compile (script.toUtf8());
    char buffer[512];
    buffer[0] = 0;

    int ret = DBAPI->tf_eval (&context, code_script, buffer, 512);
    DBAPI->tf_free (code_script);

    if (ret) {
        setWindowTitle(QString::fromUtf8(buffer));
    }


    if (trayIcon)
        trayIcon->setToolTip(QString::fromUtf8(buffer));

    if (plt)
        DBAPI->plt_unref (plt);
    if (curr_track)
        DBAPI->pl_item_unref (curr_track);
}

void MainWindow::closeEvent(QCloseEvent *e) {
    saveConfig();
    switch (actionOnClose) {
    default:
    case Exit:
        QMainWindow::closeEvent(e);
        break;
    case Hide:
        saveConfig();
        e->ignore();
        if (isHidden())
            show();
        else
            hide();
        break;
    case Minimize:
        e->ignore();
        showMinimized();
        break;
    }
}


void MainWindow::trackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    Q_UNUSED(from);
    if (to != nullptr) {
        char str[1024];
        const char *fmt;
        if (api->getOutputState() == DDB_PLAYBACK_STATE_STOPPED)
            fmt = SETTINGS->getValue("MainWindow", "TitlebarStopped", "DeaDBeeF %_deadbeef_version%").toString().toUtf8();
        else
            fmt = SETTINGS->getValue("MainWindow", "TitlebarPlaying", "%artist% - %title% - DeaDBeeF %_deadbeef_version%").toString().toUtf8();

        DBAPI->pl_item_ref(to);
        DBAPI->pl_format_title(to, 0, str, sizeof(str), -1, fmt);
        if (trayIcon) {
            // todo
            //trayIcon->showMessage("DeaDBeeF", QString::fromUtf8(str), QSystemTrayIcon::Information, 2000);
        }
        DBAPI->pl_item_unref(to);
    } else {
        //progressBar.setValue(0);
    }
    updateTitle();
}

void MainWindow::setCloseOnMinimized(bool minimizeOnClose) {
    //bool trayIconIsHidden = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::TrayIconIsHidden, false).toBool();
    //configureActionOnClose(minimizeOnClose, trayIconIsHidden);
}

void MainWindow::setTrayIconHidden(bool hideTrayIcon) {
    if (hideTrayIcon) {
        trayIcon->setVisible(false);
        delete trayIcon;
        delete trayMenu;
    }
    else {
        createTray();
    }
    
    //bool minimizeOnClose = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MinimizeOnClose, false).toBool();
    //configureActionOnClose(minimizeOnClose, hideTrayIcon);
}

void MainWindow::configureActionOnClose(bool minimizeOnClose, bool hideTrayIcon) {
    actionOnClose = Exit;
    if (minimizeOnClose) {
        if (hideTrayIcon)
            actionOnClose = Minimize;
        else
            actionOnClose = Hide;
    }    
}

// config

void MainWindow::loadConfig() {
    pl->RestoreWidgets(this);

    QSize size       = SETTINGS->getValue("MainWindow", "WindowSize", QSize(640, 480)).toSize();
    QPoint point     = SETTINGS->getValue("MainWindow", "WindowPosition", QPoint(0, 0)).toPoint();
    QByteArray state = SETTINGS->getValue("MainWindow", "WindowState", QByteArray()).toByteArray();
    // TODO
    //QByteArray state = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowState, QByteArray()).toByteArray();
    //bool mmIsHidden  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MainMenuIsHidden, false).toBool();
    //bool trayIconIsHidden = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::TrayIconIsHidden, false).toBool();
    //bool minimizeOnClose = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MinimizeOnClose, false).toBool();

    // tabs
    QStringList list = SETTINGS->getValue("MainWindow", "TabifiedDockWidgets",QStringList()).toStringList();
    if (list.length()) {
        for (int i = 0; i < list.length(); i+=2) {
            QDockWidget *dw1 = findChild<QDockWidget*>(list[i]);
            QDockWidget *dw2 = findChild<QDockWidget*>(list[i+1]);
            if (dw1 && dw2) {
                tabifyDockWidget(dw1,dw2);
                restoreState(state);
            }
        }
    }

    resize(size);
    move(point);
    restoreState(state);
    //if (!trayIconIsHidden) {
        createTray();
    //}
    //configureActionOnClose(minimizeOnClose, trayIconIsHidden);

    emit configLoaded();
}

void MainWindow::saveConfig() {
    SETTINGS->setValue("MainWindow", "WindowSize", size());
    SETTINGS->setValue("MainWindow", "WindowPosition", pos());
    SETTINGS->setValue("MainWindow", "WindowState", saveState());

    // tabs
    QList<QDockWidget*> tabs = findChildren<QDockWidget *>();
    QList<QDockWidget*> used;
    QStringList list;
    foreach(QDockWidget *dock,tabs) {
        if (!used.contains(dock)) {
            QList<QDockWidget *> l = tabifiedDockWidgets(dock);
            if (l.length()) {
                used.append(dock);
                used.append(l);
                foreach(QDockWidget *dw, l) {
                    list.append(dock->objectName());
                    list.append(dw->objectName());
                }
            }
        }
    }
    SETTINGS->setValue("MainWindow", "TabifiedDockWidgets", list);
}

void MainWindow::windowActivate() {
    if (this->isHidden())
        this->show();

    this->setWindowState( (w->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    this->raise();
    this->activateWindow();
}

void MainWindow::windowShowHide() {
    if (this->isHidden())
        this->show();
    else
        this->hide();
}

void MainWindow::on_deadbeefActivated() {
    if (isHidden()) show();
}
