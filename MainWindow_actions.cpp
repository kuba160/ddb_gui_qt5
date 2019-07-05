// PlayBack toolbar

void MainWindow::on_actionStop_triggered() {
    DBAPI->sendmessage(DB_EV_STOP, 0, 0, 0);
    updateTitle();
}

void MainWindow::on_actionPlay_triggered() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_PLAY_CURRENT);
}

void MainWindow::on_actionPause_triggered() {
    DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
}

void MainWindow::on_actionPrev_triggered() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_PREV);
}

void MainWindow::on_actionNext_triggered() {
    DBApiWrapper::Instance()->sendPlayMessage(DB_EV_NEXT);
}

// menuBar -/ File

void MainWindow::on_actionAddFiles_triggered() {
    DBFileDialog fileDialog(this,
                            tr("Add file(s) to playlist..."),
                            QStringList(),
                            QFileDialog::ExistingFiles,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void MainWindow::on_actionAddFolder_triggered() {
    DBFileDialog fileDialog(this,
                            tr("Add folder(s) to playlist..."),
                            QStringList(),
                            QFileDialog::DirectoryOnly,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void MainWindow::on_actionAddURL_triggered() {
    ui->playList->insertByURLAtPosition(QUrl::fromUserInput(QInputDialog::getText(this, tr("Enter URL..."), tr("URL: "), QLineEdit::Normal)));
}

void MainWindow::on_actionAddAudioCD_triggered() {
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, SIGNAL(finished()), ui->playList, SLOT(refresh()));
    watcher->setFuture(QtConcurrent::run(loadAudioCD));
}

// separator

// actionNewPlaylist handled by PlayList (check out MainWindow.cpp for signal connection)

void MainWindow::on_actionLoadPlaylist_triggered() {
    QStringList filters;
    filters << tr("Supported playlist formats (*.dbpl)");
    filters << tr("Other files (*)");
    DBFileDialog fileDialog(this,
                            tr("Load playlist"),
                            filters,
                            QFileDialog::ExistingFile,
                            QFileDialog::DontUseNativeDialog | QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, SIGNAL(finished()), ui->playList, SLOT(refresh()));
    watcher->setFuture(QtConcurrent::run(loadPlaylist, fileNames.last()));
}


void MainWindow::on_actionSaveAsPlaylist_triggered() {
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

    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;

    ddb_playlist_t *plt = deadbeef->plt_get_curr();
    if (plt) {
        int res = deadbeef->plt_save(plt, NULL, NULL, fileNames.last().toUtf8().constData(), NULL, NULL, NULL);
        deadbeef->plt_unref(plt);
    }
}

// separator

void MainWindow::on_actionExit_triggered() {
    actionOnClose = Exit;
    close();
}


// menuEdit

void MainWindow::on_actionClearAll_triggered() {
    ui->playList->clearPlayList();
}


void MainWindow::on_actionSelectAll_triggered() {
    ui->playList->selectAll();
}

void MainWindow::on_actionDeselectAll_triggered() {
    ui->playList->deselectAll();
}

// actionFind missing?

void MainWindow::on_actionPreferences_triggered() {
    PreferencesDialog *prefDialog = new PreferencesDialog(this);
    connect(prefDialog, SIGNAL(setCloseOnMinimize(bool)), this, SLOT(setCloseOnMinimized(bool)));
    connect(prefDialog, SIGNAL(setTrayIconHidden(bool)), this, SLOT(setTrayIconHidden(bool)));
    connect(prefDialog, SIGNAL(titlePlayingChanged()), this, SLOT(titleSettingChanged()));
    connect(prefDialog, SIGNAL(titleStoppedChanged()), this, SLOT(titleSettingChanged()));
    prefDialog->exec();
    delete prefDialog;
}

// menuView

void MainWindow::on_actionHideMenuBar_triggered() {
    ui->menuBar->setHidden(!ui->menuBar->isHidden());
    ui->actionHideMenuBar->setChecked(!ui->menuBar->isHidden());
}

void MainWindow::on_actionHideTabBar_triggered() {
    ui->playList->hideTab();
}



void MainWindow::on_actionPlayListHeader_triggered() {
    ui->playList->header();
}

/*
// todo make this dynamic
#ifdef ARTWORK_ENABLED
void MainWindow::on_actionHideCover_triggered() {
    coverArtWidget.setHidden(!coverArtWidget.isHidden());
}

void MainWindow::onCoverartClose() {
    ui->actionHideCoverArt->setChecked(false);
}

#endif
*/

void MainWindow::on_actionBlockToolbarChanges_triggered() {
    ui->PlayBackToolBar->setMovable(!ui->actionBlockToolbarChanges->isChecked());
}

// menu "Options" (called Playback)

// orderMenu

void MainWindow::on_actionLinearOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_LINEAR);
}

void MainWindow::on_actionRandomOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_RANDOM);
}

void MainWindow::on_actionShuffleOrder_triggered() {
    DBAPI->conf_set_int("playback.order", PLAYBACK_ORDER_SHUFFLE_TRACKS);
}

// loopingMenu

void MainWindow::on_actionLoopAll_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_LOOP_ALL);
}

void MainWindow::on_actionLoopTrack_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_LOOP_SINGLE);
}

void MainWindow::on_actionLoopNothing_triggered() {
    DBAPI->conf_set_int("playback.loop", PLAYBACK_MODE_NOLOOP);
}

// menu "Help"

void MainWindow::on_actionAbout_triggered() {
    AboutDialog().exec();
}

void MainWindow::on_actionAboutQt_triggered() {
    QMessageBox::aboutQt(this);
}


// tray icon

void MainWindow::trayIcon_activated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) {
        if (isHidden())
            show();
        else
            hide();
    }
    if (reason == QSystemTrayIcon::MiddleClick) {
        DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
    }

}

void MainWindow::trayIcon_wheeled(int delta) {
    volumeSlider.setValue(volumeSlider.value() + SIGNUM(delta));
}

// misc

void MainWindow::on_deadbeefActivated() {
    if (isHidden()) show();
}
