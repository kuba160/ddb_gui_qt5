#include "MainWindow.h"
#include "ui_MainWindow.h"

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
#include "PlayList.h"
#include "PreferencesDialog.h"

#include <include/callbacks.h>
#include <QtConcurrent>
#include <QFutureWatcher>
#include "DBFileDialog.h"

//#include "PluginLoader.h"

MainWindow::MainWindow(QWidget *parent, DBApi *Api) :
        QMainWindow(parent),
        DBToolbarWidget (parent, Api),
        ui(new Ui::MainWindow),
        orderGroup(this),
        loopingGroup(this) {

    ui->setupUi(this);
    
    loadActions();
    loadIcons();

    //api = new DBApi;

    orderGroup.addAction(ui->actionLinearOrder);
    orderGroup.addAction(ui->actionRandomOrder);
    orderGroup.addAction(ui->actionShuffleOrder);

    loopingGroup.addAction(ui->actionLoopAll);
    loopingGroup.addAction(ui->actionLoopTrack);
    loopingGroup.addAction(ui->actionLoopNothing);

    ui->centralwidget->hide();
    //ui->SeekToolbar->addWidget(&progressBar);
    //ui->VolumeToolbar->addWidget(&volumeSlider);
    //QWidget *empty = new QWidget(this);
    //empty->setVisible(false);
    //this->setCentralWidget(empty);
    //ui->mainLayout->addWidget(&playList);

    new_plugins = ui->menuView->addMenu("New...");
    remove_plugins = ui->menuView->addMenu("Remove...");
    remove_plugins->menuAction()->setVisible(false);

    //// PluginLoader
    // Create links for widget creation
    connect (pl,SIGNAL(toolBarCreated(QToolBar *)),this,SLOT(windowAddToolbar(QToolBar *)));
    connect (pl, SIGNAL(dockableWidgetCreated(QDockWidget *)), this, SLOT(windowAddDockable(QDockWidget *)));

    connect (pl, SIGNAL(actionToggleVisibleCreated(QAction *)), this, SLOT(windowViewActionAdd(QAction *)));

    // New widget creation
    {
        // add already loaded widgets
        unsigned int i = 0;
        QAction *a;
        for (i = 0; (a = pl->actionNewGet(i)); i++) {
            new_plugins->addAction(a);
        }
        // subscribe for future actions
        connect (pl, SIGNAL(actionPluginAddCreated(QAction *)), this, SLOT(windowViewActionCreate(QAction *)));
    }

    // Deletion of existent plugins
    {
        // subscribe for future actions
        connect (pl, SIGNAL(actionPluginRemoveCreated(QAction *)), this, SLOT(windowViewActionRemove(QAction *)));
    }

    connect (this, SIGNAL(configLoaded()), pl, SLOT(updateActionChecks()));

    connect (this->ui->actionBlockToolbarChanges, SIGNAL(toggled(bool)), pl, SLOT(lockWidgets(bool)));

    connect (this->ui->actionExit, SIGNAL(triggered()), pl, SLOT(actionChecksSave()));

    trayIcon = nullptr;
    trayMenu = nullptr;
    
    createConnections();

    pl->RestoreWidgets(this);

    loadConfig();
    pl->lockWidgets(ui->actionBlockToolbarChanges->isChecked());
    updateTitle();
}

MainWindow::~MainWindow() {
    saveConfig();
    delete trayIcon;
    delete ui;
}

DBApi* MainWindow::Api() {
    return api;
}

void MainWindow::windowAddToolbar(QToolBar *toolbar) {
    toolbar->setParent(this);
    this->addToolBar(toolbar);
    // reload config
    //loadConfig();
}

void MainWindow::windowAddDockable(QDockWidget *dock) {
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::windowViewActionAdd(QAction *action) {
    this->ui->menuView->addAction(action);
}

void MainWindow::windowViewActionCreate(QAction *action) {
    new_plugins->addAction(action);
}

void MainWindow::windowViewActionRemove(QAction *action) {
    remove_plugins->addAction(action);
    remove_plugins->menuAction()->setVisible(true);
}

void MainWindow::windowViewActionRemoveToggleHide(bool visible) {
    remove_plugins->menuAction()->setVisible(visible);
}


void MainWindow::createConnections() {
    connect(api, SIGNAL(trackChanged(DB_playItem_t*,DB_playItem_t*)), this, SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    //connect(ui->actionNewPlaylist, SIGNAL(triggered()), ui->playList, SIGNAL(newPlaylist()));
    connect(api, SIGNAL(deadbeefActivated()), this, SLOT(on_deadbeefActivated()));
}

void MainWindow::loadIcons() {
    ui->actionPlay->setIcon(getStockIcon(this, "media-playback-start", QStyle::SP_MediaPlay));
    ui->actionPause->setIcon(getStockIcon(this, "media-playback-pause", QStyle::SP_MediaPause));
    ui->actionStop->setIcon(getStockIcon(this, "media-playback-stop", QStyle::SP_MediaStop));
    ui->actionPrev->setIcon(getStockIcon(this, "media-skip-backward", QStyle::SP_MediaSkipBackward));
    ui->actionNext->setIcon(getStockIcon(this, "media-skip-forward", QStyle::SP_MediaSkipForward));
    ui->actionExit->setIcon(getStockIcon(this, "application-exit", QStyle::SP_DialogCloseButton));
    ui->actionClearAll->setIcon(getStockIcon(this, "edit-clear", QStyle::SP_TrashIcon));
    ui->actionAddFiles->setIcon(getStockIcon(this, "document-open", QStyle::SP_FileIcon));
    ui->actionAddFolder->setIcon(getStockIcon(this, "folder-m", QStyle::SP_DirIcon));
    ui->actionAddURL->setIcon(getStockIcon(this, "folder-remote", QStyle::SP_DriveNetIcon));
    ui->actionAddAudioCD->setIcon(getStockIcon(this, "media-optical-audio", QStyle::SP_DriveCDIcon));
    ui->actionNewPlaylist->setIcon(getStockIcon(this, "document-new", QStyle::SP_FileDialogNewFolder));
    ui->actionPreferences->setIcon(getStockIcon(this, "preferences-system", QStyle::SP_CustomBase));
    ui->actionAbout->setIcon(getStockIcon(this, "help-about", QStyle::SP_DialogHelpButton));

    //pauseIcon = getStockIcon(this, "media-playback-pause", QStyle::SP_MediaPause);
    //playIcon = getStockIcon(this, "media-playback-start", QStyle::SP_MediaPlay);
}

void MainWindow::loadActions() {
    addAction(ui->actionAddFolder);
    addAction(ui->actionExit);
    addAction(ui->actionPreferences);
    addAction(ui->actionAddFiles);
    addAction(ui->actionAddURL);
    addAction(ui->actionSaveAsPlaylist);
    addAction(ui->actionLoadPlaylist);
    addAction(ui->actionNewPlaylist);
    addAction(ui->actionHideMenuBar);
    addAction(ui->actionFind);
}

void MainWindow::createTray() {
    trayIcon = new SystemTrayIcon(this);
    trayMenu = new QMenu(this);
    trayMenu->addAction(ui->actionPlay);
    trayMenu->addAction(ui->actionPause);
    trayMenu->addAction(ui->actionStop);
    trayMenu->addAction(ui->actionNext);
    trayMenu->addAction(ui->actionPrev);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->actionExit);
    trayIcon->setContextMenu(trayMenu);

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

void MainWindow::updateTitle(DB_playItem_t *it) {

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
    const char * script = nullptr;
    if (api->getOutputState() == DDB_PLAYBACK_STATE_STOPPED)
        script = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarStopped, "DeaDBeeF %_deadbeef_version%").toString().toUtf8().constData();
    else
        script = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarPlaying, "%artist% - %title% -DeaDBeeF %_deadbeef_version%").toString().toUtf8().constData();

    char * code_script = DBAPI->tf_compile (script);
    char buffer[256];

    DBAPI->tf_eval (&context, code_script, buffer, 256);
    DBAPI->tf_free (code_script);

    setWindowTitle(QString::fromUtf8(buffer));


    if (trayIcon)
        trayIcon->setToolTip(QString::fromUtf8(buffer));

    if (plt)
        DBAPI->plt_unref (plt);
    if (curr_track)
        DBAPI->pl_item_unref (curr_track);
}

void MainWindow::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *e) {
    switch (actionOnClose) {
    case Exit:
        e->accept();
        break;
    case Hide:
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
    if (to != nullptr) {
        char str[1024];
        const char *fmt = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::MessageFormat, "%a - %t").toString().toUtf8().constData();
        DBAPI->pl_item_ref(to);
        DBAPI->pl_format_title(to, 0, str, sizeof(str), -1, fmt);
        bool showTrayTips = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::ShowTrayTips, false).toBool();
        if (trayIcon && showTrayTips) {
            trayIcon->showMessage("DeaDBeeF", QString::fromUtf8(str), QSystemTrayIcon::Information, 2000);
        }
        DBAPI->pl_item_unref(to);
    } else {
        //progressBar.setValue(0);
    }
    updateTitle(to);
}



void MainWindow::createToolBars() {

}

QMenu *MainWindow::createPopupMenu() {
    QMenu *popupMenu = new QMenu();
    popupMenu->addAction(ui->actionHideMenuBar);
    //popupMenu->addSeparator();
    //popupMenu->addSeparator();
    //popupMenu->addSeparator();
    //popupMenu->addAction(ui->actionBlockToolbarChanges);
    return popupMenu;
}

void MainWindow::setCloseOnMinimized(bool minimizeOnClose) {
    bool trayIconIsHidden = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::TrayIconIsHidden, false).toBool();
    configureActionOnClose(minimizeOnClose, trayIconIsHidden);
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
    
    bool minimizeOnClose = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MinimizeOnClose, false).toBool();
    configureActionOnClose(minimizeOnClose, hideTrayIcon);
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
    QSize size       = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowSize, QSize(640, 480)).toSize();
    QPoint point     = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowPosition, QPoint(0, 0)).toPoint();
    QByteArray state = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowState, QByteArray()).toByteArray();
    bool tbIsLocked  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::ToolbarsIsLocked, false).toBool();
    bool mmIsHidden  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MainMenuIsHidden, false).toBool();
    bool trayIconIsHidden = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::TrayIconIsHidden, false).toBool();
    bool minimizeOnClose = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MinimizeOnClose, false).toBool();
    bool headerIsVisible = SETTINGS->getValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderIsVisible,true).toBool();
    bool tbIsVisible = SETTINGS->getValue(QtGuiSettings::MainWindow,QtGuiSettings::TabBarIsVisible,true).toBool();

    resize(size);
    move(point);
    ui->actionBlockToolbarChanges->setChecked(tbIsLocked);
    menuBar()->setHidden(mmIsHidden);
    ui->actionHideMenuBar->setChecked(!menuBar()->isHidden());

    ui->actionPlayListHeader->setChecked(headerIsVisible);
    ui->actionHideTabBar->setChecked(tbIsVisible);

    if (!trayIconIsHidden) {
        createTray();
    }
    
    bool caIsHidden  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::CoverartIsHidden, false).toBool();
    ui->actionHideCoverArt->setChecked(!caIsHidden);
   // if (ui->actionHideCoverArt->isChecked()) {
    //    addDockWidget(Qt::LeftDockWidgetArea, &coverArtWidget);
        //connect(&coverArtWidget, SIGNAL(onCloseEvent()), this, SLOT(onCoverartClose()));
   // }
    // when no coverart make it not visible
    //ui->actionHideCoverArt->setVisible(false);

    restoreState(state);
    configureActionOnClose(minimizeOnClose, trayIconIsHidden);

    //int i;
    //for (i = 0; i < ToolbarStackCount; i++) {
    //    ToolbarStack[i]->setMovable(!ui->actionBlockToolbarChanges->isChecked());
    //}

    switch (DBAPI->conf_get_int("playback.order", PLAYBACK_ORDER_LINEAR)) {
    case PLAYBACK_ORDER_LINEAR:
        ui->actionLinearOrder->setChecked(true);
        break;
    case PLAYBACK_ORDER_RANDOM:
        ui->actionRandomOrder->setChecked(true);
        break;
    case PLAYBACK_ORDER_SHUFFLE_TRACKS:
        ui->actionShuffleOrder->setChecked(true);
        break;
    }

    switch (DBAPI->conf_get_int("playback.loop", PLAYBACK_MODE_NOLOOP)) {
    case PLAYBACK_MODE_LOOP_ALL:
        ui->actionLoopAll->setChecked(true);
        break;
    case PLAYBACK_MODE_LOOP_SINGLE:
        ui->actionLoopTrack->setChecked(true);
        break;
    case PLAYBACK_MODE_NOLOOP:
        ui->actionLoopNothing->setChecked(true);
        break;
    }
    emit configLoaded();

}

void MainWindow::saveConfig() {
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowSize, size());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowPosition, pos());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowState, saveState());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::ToolbarsIsLocked, ui->actionBlockToolbarChanges->isChecked());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::MainMenuIsHidden, menuBar()->isHidden());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::CoverartIsHidden, !ui->actionHideCoverArt->isChecked());
    //playList.saveConfig();
    //pl->actionChecksSave();
}

void MainWindow::on_actionRemove_triggered() {
    //playList.deleteSelectedTracks();
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
