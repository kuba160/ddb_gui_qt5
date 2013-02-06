#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

#include "QtGuiSettings.h"

#include "QtGui.h"
#include "DBApiWrapper.h"
#include "AboutDialog.h"
#include "PlayList.h"
#include "PreferencesDialog.h"

#define SIGNUM(x) ((x > 0) - (x < 0))

#include <include/callbacks.h>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include "DBFileDialog.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        orderGroup(this),
#ifdef ARTWORK_ENABLED
        coverArtWidget(this),
#endif
        loopingGroup(this),
        volumeSlider(this),
        progressBar(this) {

    ui->setupUi(this);

    loadIcons();
    createToolBars();
    
    dbStatusBar = NULL;
    trayIcon = NULL;
    trayMenu = NULL;
    
    createStatusbar();
    
    createConnections();

    loadConfig();
    updateTitle();
}

MainWindow::~MainWindow() {
    saveConfig();
    delete trayIcon;
    delete ui;
}

void MainWindow::createConnections() {
    connect(DBApiWrapper::Instance(), SIGNAL(trackChanged(DB_playItem_t*,DB_playItem_t*)), this, SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    connect(ui->actionNewPlaylist, SIGNAL(triggered()), ui->playList, SIGNAL(newPlaylist()));
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

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/root/images/scalable.svg"), QSize(), QIcon::Normal, QIcon::On);
    trayIcon->setIcon(icon);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIcon_activated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon, SIGNAL(wheeled(int)), this, SLOT(trayIcon_wheeled(int)));
    
    trayIcon->setVisible(true);
}

void MainWindow::createStatusbar() {
    setStatusBar(dbStatusBar);
}

void MainWindow::titleSettingChanged() {
    updateTitle();
}

void MainWindow::updateTitle(DB_playItem_t *it) {
    char str[256];
    const char *fmt;

    if (!it)
        it = DBAPI->streamer_get_playing_track();
    else
        DBAPI->pl_item_ref(it);

    if (it)
        fmt = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarPlaying, "%a - %t - DeaDBeeF-%V").toString().toUtf8().constData();
    else
        fmt = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarStopped, "DeaDBeeF-%V").toString().toUtf8().constData();

    DBAPI->pl_format_title(it, -1, str, sizeof(str), -1, fmt);

    setWindowTitle(QString::fromUtf8(str));
    if (trayIcon)
        trayIcon->setToolTip(QString::fromUtf8(str));

    if (it)
        DBAPI->pl_item_unref(it);
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
        WRAPPER->Destroy();
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

void MainWindow::on_actionAddFolder_activated() {
    DBFileDialog fileDialog(this,
                            tr("Add folder(s) to playlist..."),
                            QStringList(),
                            QFileDialog::DirectoryOnly,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void MainWindow::on_actionClearAll_activated() {
    ui->playList->clearPlayList();
}

void MainWindow::on_actionPlay_activated() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_PLAY_CURRENT);
}

void MainWindow::on_actionStop_activated() {
    DBAPI->sendmessage(DB_EV_STOP, 0, 0, 0);
    updateTitle();
}

void MainWindow::on_actionNext_activated() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_NEXT);
}

void MainWindow::on_actionPrev_activated() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_PREV);
}

void MainWindow::on_actionPause_activated() {
    DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
}

void MainWindow::trayIcon_activated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isHidden())
            show();
        else
            hide();
    }
}

void MainWindow::on_actionExit_activated() {
    if (dbStatusBar != NULL) {
        delete dbStatusBar;
        dbStatusBar = NULL;
    }
    actionOnClose = Exit;
    close();
}

void MainWindow::trayIcon_wheeled(int delta) {
    volumeSlider.setValue(volumeSlider.value() + SIGNUM(delta));
}

void MainWindow::trackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    if (to != NULL) {
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
        progressBar.setValue(0);
    }
    updateTitle(to);
}


void MainWindow::on_actionLinearOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_LINEAR);
}

void MainWindow::on_actionRandomOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_RANDOM);
}

void MainWindow::on_actionShuffleOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_SHUFFLE_TRACKS);
}

void MainWindow::on_actionLoopAll_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_LOOP_ALL);
}

void MainWindow::on_actionLoopTrack_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_LOOP_SINGLE);
}

void MainWindow::on_actionLoopNothing_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_NOLOOP);
}

void MainWindow::on_actionAbout_triggered() {
    AboutDialog().exec();
}

void MainWindow::on_actionAboutQt_triggered() {
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionPreferences_triggered() {
    PreferencesDialog *prefDialog = new PreferencesDialog(this);
    connect(prefDialog, SIGNAL(setCloseOnMinimize(bool)), this, SLOT(setCloseOnMinimized(bool)));
    connect(prefDialog, SIGNAL(setTrayIconHidden(bool)), this, SLOT(setTrayIconHidden(bool)));
    connect(prefDialog, SIGNAL(titlePlayingChanged()), this, SLOT(titleSettingChanged()));
    connect(prefDialog, SIGNAL(titleStoppedChanged()), this, SLOT(titleSettingChanged()));
    prefDialog->exec();
    delete prefDialog;
}

void MainWindow::on_actionSelectAll_activated() {
    ui->playList->selectAll();
}

void MainWindow::on_actionDeselectAll_activated() {
    ui->playList->deselectAll();
}

void MainWindow::on_actionRemove_activated() {
    ui->playList->deleteSelectedTracks();
}

void MainWindow::on_actionAddFiles_activated() {
    DBFileDialog fileDialog(this,
                            tr("Add file(s) to playlist..."),
                            QStringList(),
                            QFileDialog::ExistingFiles,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void MainWindow::on_actionAddAudioCD_activated() {
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, SIGNAL(finished()), ui->playList, SLOT(refresh()));
    watcher->setFuture(QtConcurrent::run(loadAudioCD));
}

void MainWindow::on_actionAddURL_activated() {
    ui->playList->insertByURLAtPosition(QUrl::fromUserInput(QInputDialog::getText(this, tr("Enter URL..."), tr("URL: "), QLineEdit::Normal)));
}

void MainWindow::createToolBars() {
    orderGroup.addAction(ui->actionLinearOrder);
    orderGroup.addAction(ui->actionRandomOrder);
    orderGroup.addAction(ui->actionShuffleOrder);

    loopingGroup.addAction(ui->actionLoopAll);
    loopingGroup.addAction(ui->actionLoopTrack);
    loopingGroup.addAction(ui->actionLoopNothing);

    ui->PlayBackToolBar->addWidget(&progressBar);
    ui->PlayBackToolBar->addWidget(&volumeSlider);
}

QMenu *MainWindow::createPopupMenu() {
    QMenu *popupMenu = new QMenu();
    popupMenu->addAction(ui->actionHideMenuBar);
    popupMenu->addSeparator();
    popupMenu->addSeparator();
    popupMenu->addAction(ui->actionHideStatusbar);
    popupMenu->addSeparator();
    popupMenu->addAction(ui->actionBlockToolbarChanges);
    return popupMenu;
}

void MainWindow::on_actionHideStatusbar_activated() {
    if (statusBar()->isHidden()) {
        dbStatusBar = new StatusBar(this);
        statusBar()->setHidden(false);
        setStatusBar(dbStatusBar);
    }
    else {
        setStatusBar(NULL);
        statusBar()->setHidden(true);
        delete dbStatusBar;
        dbStatusBar = NULL;
    }
}

void MainWindow::on_actionHideMenuBar_activated() {
    on_actionToggleHideMenu_triggered();
}

void MainWindow::on_actionBlockToolbarChanges_activated() {
    ui->PlayBackToolBar->setMovable(!ui->actionBlockToolbarChanges->isChecked());
}

void MainWindow::on_actionSaveAsPlaylist_activated() {
    QStringList filters;
    filters << tr("DeaDBeeF playlist files (*.dbpl)");
    DB_playlist_t **plug = deadbeef->plug_get_playlist_list();
    for (int i = 0; plug[i]; i++) {
        if (plug[i]->extensions && plug[i]->load) {
            const char **exts = plug[i]->extensions;
            if (exts && plug[i]->save)
                for (int e = 0; exts[e]; e++)
                    filters << QString("*.%1").arg(exts[e]);
        }
    }
    DBFileDialog fileDialog(this,
                            tr("Save playlist as..."),
                            filters,
                            QFileDialog::AnyFile,
                            QFileDialog::DontUseNativeDialog);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    
    QStringList fileNames = fileDialog.exec();
    if (fileNames.isEmpty())
        return;
    
    ddb_playlist_t *plt = deadbeef->plt_get_curr();
    if (plt) {
        int res = deadbeef->plt_save(plt, NULL, NULL, fileNames.last().toUtf8().constData(), NULL, NULL, NULL);
        deadbeef->plt_unref(plt);
    }
}

void MainWindow::on_actionLoadPlaylist_activated() {
    QStringList filters;
    filters << tr("Supported playlist formats (*.dbpl)");
    filters << tr("Other files (*)");
    DBFileDialog fileDialog(this,
                            tr("Load playlist"),
                            filters,
                            QFileDialog::ExistingFile,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec();
    if (fileNames.isEmpty())
        return;

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, SIGNAL(finished()), ui->playList, SLOT(refresh()));
    watcher->setFuture(QtConcurrent::run(loadPlaylist, fileNames.last()));
}

#ifdef ARTWORK_ENABLED
void MainWindow::on_actionHideCoverArt_activated() {
    coverArtWidget.setHidden(!coverArtWidget.isHidden());
}

void MainWindow::onCoverartClose() {
    ui->actionHideCoverArt->setChecked(false);
}

#endif

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

void MainWindow::loadConfig() {
    QSize size       = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowSize, QSize(640, 480)).toSize();
    QPoint point     = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowPosition, QPoint(0, 0)).toPoint();
    QByteArray state = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowState, QByteArray()).toByteArray();
    bool tbIsLocked  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::ToolbarsIsLocked, false).toBool();
    bool mmIsHidden  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MainMenuIsHidden, false).toBool();
    bool sbIsHidden  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::StatusbarIsHidden, false).toBool();
    bool trayIconIsHidden = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::TrayIconIsHidden, false).toBool();
    bool minimizeOnClose = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MinimizeOnClose, false).toBool();
    bool isPlayListVisible = SETTINGS->getValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderIsVisible,true).toBool();
    bool isTabBarVisible = SETTINGS->getValue(QtGuiSettings::MainWindow,QtGuiSettings::TabBarIsVisible,true).toBool();

    resize(size);
    move(point);
    ui->actionBlockToolbarChanges->setChecked(tbIsLocked);
    menuBar()->setHidden(mmIsHidden);
    ui->actionHideMenuBar->setChecked(!menuBar()->isHidden());

    ui->actionPlayListHeader->setChecked(isPlayListVisible);
    ui->actionHideTabBar->setChecked(isTabBarVisible);

    dbStatusBar = sbIsHidden ? NULL : new StatusBar(this);
    setStatusBar(dbStatusBar);
    statusBar()->setHidden(sbIsHidden);
    ui->actionHideStatusbar->setChecked(!sbIsHidden);

    if (!trayIconIsHidden) {
        createTray();
    }
    
#ifdef ARTWORK_ENABLED
    bool caIsHidden  = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::CoverartIsHidden, false).toBool();
    ui->actionHideCoverArt->setChecked(!caIsHidden);
    if (ui->actionHideCoverArt->isChecked()) {
        addDockWidget(Qt::LeftDockWidgetArea, &coverArtWidget);
        connect(&coverArtWidget, SIGNAL(onCloseEvent()), this, SLOT(onCoverartClose()));
    }
#else
    ui->actionHideCoverArt->setVisible(false);
#endif

    restoreState(state);
    configureActionOnClose(minimizeOnClose, trayIconIsHidden);

    ui->PlayBackToolBar->setMovable(!ui->actionBlockToolbarChanges->isChecked());

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

    addAction(ui->actionToggleHideMenu);


    qDebug() << QString::fromUtf8(DEADBEEF_PREFIX);
}

void MainWindow::saveConfig() {
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowSize, size());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowPosition, pos());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::WindowState, saveState());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::ToolbarsIsLocked, ui->actionBlockToolbarChanges->isChecked());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::MainMenuIsHidden, menuBar()->isHidden());
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::StatusbarIsHidden, statusBar()->isHidden());
#ifdef ARTWORK_ENABLED
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::CoverartIsHidden, !ui->actionHideCoverArt->isChecked());
#endif
    ui->playList->saveConfig();
}

void MainWindow::on_actionPlayListHeader_triggered() {
    ui->playList->header();
}

void MainWindow::on_actionHideTabBar_triggered() {
    ui->playList->hideTab();
}

void MainWindow::on_actionToggleHideMenu_triggered() {
    ui->menuBar->setHidden(!ui->menuBar->isHidden());
}
