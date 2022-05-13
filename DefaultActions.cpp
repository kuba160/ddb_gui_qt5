#include "DefaultActions.h"
#include "ui_DefaultActions.h"
#include <QObject>
#include <QStyle>

#include "AboutDialog.h"
#include "PluginLoader.h"
#include "QtGui.h"
#include "QtGuiSettings.h"
#include "DBFileDialog.h"


DefaultActions::DefaultActions(DBApi *Api, QWidget *parent) : QWidget(parent), DBWidget(parent,Api), ui(new Ui::DefaultActions) {
    ui->setupUi(this);

    // Shuffle/Repeat connections
    {
        shuffleGroup = new QActionGroup(this);
        repeatGroup  = new QActionGroup(this);
        shuffle[0] = ui->actionNoShuffle;
        shuffle[1] = ui->actionTrackShuffle;
        shuffle[2] = ui->actionAlbumShuffle;
        shuffle[3] = ui->actionRandomTrackShuffle;
        repeat[0]  = ui->actionLoopAll;
        repeat[1]  = ui->actionLoopTrack;
        repeat[2]  = ui->actionLoopNothing;
        int i;
        for (i = 0; i < 4; i++) {
            connect(shuffle[i],SIGNAL(triggered()),this, SLOT(shuffleRepeatHandler()));
            shuffleGroup->addAction(shuffle[i]);
        }
        for (i = 0; i < 3; i++) {
            connect(repeat[i],SIGNAL(triggered()),this, SLOT(shuffleRepeatHandler()));
            repeatGroup->addAction(repeat[i]);
        }
        // swap repeat order
        {QAction *c = repeat[1]; repeat[1] = repeat[2]; repeat[2] = c;}
        // Restore
        shuffle[DBAPI->streamer_get_shuffle()]->setChecked(true);
        repeat[DBAPI->streamer_get_repeat()]->setChecked(true);
    }

    main_widgets = ui->menuView->addMenu(tr("Main Widget"));
    main_widgets->setIcon(QIcon::fromTheme("bookmark-new"));
    main_widgets_list = new QActionGroup(this);
    new_plugins = ui->menuView->addMenu(QString("%1...") .arg(tr("Add")));
    new_plugins->setIcon(QIcon::fromTheme("list-add"));
    remove_plugins = ui->menuView->addMenu(QString("%1...") .arg(tr("Remove")));
    remove_plugins->setIcon(QIcon::fromTheme("list-remove"));
    ui->menuView->addSeparator();
    //remove_plugins->menuAction()->setVisible(false);

    // New widget creation menu
    {
        QList<DBWidgetInfo*> wilist = pl->getWidgetLibrary();

        QHash<QString, QAction*> acts_map;
        QStringList acts;
        foreach (DBWidgetInfo *wi, wilist) {
            QAction *a = new QAction(wi->friendlyName, new_plugins);
            a->setProperty("internalName", wi->internalName);
            connect (a, SIGNAL(triggered()), this, SLOT(onWidgetAdd()));
            acts_map.insert(wi->friendlyName, a);
            acts.append(wi->friendlyName);
        }
        acts.sort();
        foreach (QString act, acts) {
            new_plugins->addAction(acts_map.value(act));
        }
        // subscribe for future actions
        connect (pl, SIGNAL(widgetLibraryAdded(DBWidgetInfo)), this, SLOT(onWidgetLibraryAdded(DBWidgetInfo)));
        connect (pl, SIGNAL(widgetAdded(int)), this, SLOT(onWidgetAdded(int)));
    }

    QList<DBWidgetInfo>* wilist = pl->getWidgets();
    // Deletion of existent plugins
    {

        foreach (DBWidgetInfo wi, *wilist) {
            qDebug() << wi.internalName;
            QAction *a = remove_plugins->addAction(wi.friendlyName);
            a->setProperty("internalName", wi.internalName);
            connect (a, SIGNAL(triggered()), this, SLOT(onWidgetRemove()));
        }

        // subscribe for future actions
        connect (pl, SIGNAL(widgetRemoved(QString)), this, SLOT(onWidgetRemoved(QString)));
    }

    main_widgets->menuAction()->setVisible(false);
    foreach (DBWidgetInfo wi, *wilist) {
        if (wi.type == DBWidgetInfo::TypeMainWidget) {
             main_widgets->menuAction()->setVisible(true);
             QAction *a = main_widgets_list->addAction(wi.friendlyName);
             a->setProperty("internalName", wi.internalName);
             main_widgets->addAction(a);
             connect (a, SIGNAL(triggered()), this, SLOT(onWidgetMain()));
        }
    }
    main_widgets_list->setExclusive(true);


    delete wilist;

    // MainWidget selection
    //connect (pl, SIGNAL(actionPluginMainWidgetCreated(QAction *)), this, SLOT(onMainWidgetAdded(QAction *)));

    //connect (this, SIGNAL(configLoaded()), pl, SLOT(updateActionChecks()));
    ui->actionBlockToolbarChanges->setChecked(settings->getValue("PluginLoader", "designMode", false).toBool());
    connect (this->ui->actionBlockToolbarChanges, SIGNAL(toggled(bool)), pl, SLOT(setDesignMode(bool)));

    //connect (this->ui->actionExit, SIGNAL(triggered()), pl, SLOT(actionChecksSave()));

    //connect (pl, SIGNAL(actionToggleVisibleCreated(QAction *)), this, SLOT(onActionToggleCreated(QAction *)));

    ui->actionScrollPlayback->setChecked(DBAPI->conf_get_int("playlist.scroll.followplayback", true));
    ui->actionCursorPlayback->setChecked(DBAPI->conf_get_int("playlist.scroll.cursorfollowplayback", true));
    ui->actionStopTrack->setChecked(DBAPI->conf_get_int("playlist.stop_after_current", false));
    ui->actionStopAlbum->setChecked(DBAPI->conf_get_int("playlist.stop_after_album", false));
    // watch stop track/album
    connect (api, SIGNAL(trackChanged()), this, SLOT(onTrackChanged()));
}

QMenuBar *DefaultActions::getDefaultMenuBar() {
    return ui->menuBar;
}

void DefaultActions::shuffleRepeatHandler() {
    QObject *s = sender();
    int i;
    for (i = 0; i < 4; i++) {
        if (s == shuffle[i]) {
            api->setShuffle(static_cast<ddb_shuffle_t>(i));
            return;
        }
    }
    for (i = 0; i < 3; i++) {
        if (s == repeat[i]) {
            api->setRepeat(static_cast<ddb_repeat_t>(i));
            return;
        }
    }
    qDebug() << "MainWindow: shuffleRepeatHandler failed!";
}

void DefaultActions::onWidgetAdd() {
    QObject* emitter = sender();
    pl->addWidget(emitter->property("internalName").toString());
}

void DefaultActions::onWidgetRemove() {
    QObject* emitter = sender();
    pl->removeWidget(emitter->property("internalName").toString());
}

void DefaultActions::onWidgetMain() {
    QObject* emitter = sender();
    pl->setMainWidget(emitter->property("internalName").toString());
    qobject_cast<QAction *>(emitter)->setChecked(true);
}

void DefaultActions::onWidgetToggle(bool toggle) {
    QObject* emitter = sender();
    pl->setVisible(emitter->property("internalName").toString(), toggle);
}

void DefaultActions::onWidgetAdded(int num) {
    DBWidgetInfo info = pl->getWidgetAt(num);
    // add view/hide
    QAction *a = ui->menuView->addAction(info.friendlyName);
    a->setCheckable(true);
    a->setChecked(api->confGetValue("PluginLoader",QString("%1/visible") .arg(info.internalName),true).toBool());
    a->setProperty("internalName", info.internalName);
    connect (a, SIGNAL(triggered(bool)), this, SLOT(onWidgetToggle(bool)));

    // add in remove
    a = remove_plugins->addAction(info.friendlyName);
    a->setProperty("internalName", info.internalName);
    connect (a, SIGNAL(triggered()), this, SLOT(onWidgetRemove()));

    // add in main widget selection (if mainwidget)
    if (info.type == DBWidgetInfo::TypeMainWidget) {
        main_widgets->menuAction()->setVisible(true);
        a = main_widgets_list->addAction(info.friendlyName);
        a->setProperty("internalName", info.internalName);
        a->setCheckable(true);
        if(info.internalName == api->confGetValue("PluginLoader","MainWidget","playlist").toString()) {
            a->setChecked(true);
        }
        main_widgets->addAction(a);
        connect (a, SIGNAL(triggered()), this, SLOT(onWidgetMain()));
    }
}

void DefaultActions::onWidgetRemoved(QString name) {
    // TODO regenerate remove list and hide list
    QList<QAction *>list_rp = remove_plugins->actions();
    QList<QAction *>list_mw = main_widgets_list->actions();
    QList<QAction *>list_mm = ui->menuView->actions();
    int i;
    // Remove widget menu
    for (i = 0; i < list_rp.length(); i++) {
        QAction *a = list_rp.at(i);
        if (a->property("internalName").toString() == name) {
            list_rp.takeAt(i);
            delete a;
            break;
        }
    }
    // Main widget menu
    for (i = 0; i < list_mw.length(); i++) {
        QAction *a = list_mw.at(i);
        if (a->property("internalName").toString() == name) {
            list_mw.takeAt(i);
            delete a;
            if (list_mw.length() == 0) {
                main_widgets->menuAction()->setVisible(false);
            }
            break;
        }
    }
    // Show hide menu (main)
    for (i = 0; i < list_mm.length(); i++) {
        QAction *a = list_mm.at(i);
        if (a->property("internalName").toString() == name) {
            list_mm.takeAt(i);
            delete a;
            break;
        }
    }
}

void DefaultActions::onWidgetLibraryAdded(DBWidgetInfo i) {

    // fix sorting
    /*
    QList<QAction*> acts = new_plugins.actions();
    QList<DBWidgetInfo*> wilist = pl->getWidgetLibrary();
    QHash<QString, QAction*> acts_map;
    QStringList acts;
    int i;
    foreach (DBWidgetInfo *wi, wilist) {

        if (new_widget.friendlyName >)
        acts_map.insert(wi->friendlyName, a);
        acts.append(wi->friendlyName);
    }
    acts.sort();
    */
    QAction *a = new QAction(i.friendlyName, new_plugins);
    a->setProperty("internalName", i.internalName);
    connect (a, SIGNAL(triggered()), this, SLOT(onWidgetAdd()));
    new_plugins->addAction(a);
}

void DefaultActions::onTrackChanged() {
    ui->actionStopTrack->setChecked(DBAPI->conf_get_int("playlist.stop_after_current", false));
    ui->actionStopAlbum->setChecked(DBAPI->conf_get_int("playlist.stop_after_album", false));
}


void DefaultActions::on_actionAboutQt_triggered() {
    QMessageBox::aboutQt(this);
}

void DefaultActions::on_actionAbout_triggered() {
    AboutDialog dialog(this,api);
    dialog.exec();
}

void DefaultActions::on_actionPreferences_triggered() {

}

void DefaultActions::on_actionExit_triggered() {
    w->close();
}

void DefaultActions::on_actionScrollPlayback_triggered(bool checked) {
    DBAPI->conf_set_int ("playlist.scroll.followplayback", checked);
    DBAPI->sendmessage (DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void DefaultActions::on_actionCursorPlayback_triggered(bool checked) {
    DBAPI->conf_set_int ("playlist.scroll.cursorfollowplayback", checked);
    DBAPI->sendmessage (DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void DefaultActions::on_actionStopTrack_triggered(bool checked) {
    DBAPI->conf_set_int ("playlist.stop_after_current", checked);
    DBAPI->sendmessage (DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void DefaultActions::on_actionStopAlbum_triggered(bool checked) {
    DBAPI->conf_set_int ("playlist.stop_after_album", checked);
    DBAPI->sendmessage (DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void DefaultActions::on_actionJump_to_current_track_triggered() {
    api->changePlaylist(DBAPI->streamer_get_current_playlist());
    emit api->jumpToCurrentTrack();
}

void DefaultActions::on_actionOpenFiles_triggered() {
    DBFileDialog *fd = new DBFileDialog(this, DBFileDialog::OPEN_FILES);
    fd->open();
    /*DBFileDialog fileDialog(this,
                            tr("Open file(s)..."),
                            QStringList(),
                            QFileDialog::ExistingFiles,
                            QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty()) {
        return;
    }
    foreach (QString localFile, fileNames) {
        api->addTracksByUrl(localFile, DBAPI->pl_getcount(PL_MAIN) - 1);
    }*/
}



void DefaultActions::on_actionNewPlaylist_triggered() {
    api->newPlaylist(tr("New Playlist"));
}

void DefaultActions::on_actionLoadPlaylist_triggered() {
    DBFileDialog *fd = new DBFileDialog(this, DBFileDialog::LOAD_PLAYLIST);
    fd->open();
    /*foreach (QString localFile, fileNames) {
        api->loadPlaylist(localFile);
    }*/
}

void DefaultActions::on_actionSaveAsPlaylist_triggered() {
    /*
    QStringList filters;
    filters << tr("DeaDBeeF playlist files (*.dbpl)");
    DB_playlist_t **plug = DBAPI->plug_get_playlist_list();
    for (int i = 0; plug[i]; i++) {
        if (plug[i]->extensions && plug[i]->load) {
            const char **exts = plug[i]->extensions;
            if (exts && plug[i]->save)
                for (int e = 0; exts[e]; e++)
                    filters << QString("*.%1").arg(exts[e]);
        }
    }*/
    DBFileDialog *fd = new DBFileDialog(this, DBFileDialog::SAVE_PLAYLIST);
    fd->open();

                            /**
                            filters,
                            QFileDialog::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;

    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    if (plt) {
        int res = DBAPI->plt_save(plt, NULL, NULL, fileNames.last().toUtf8().constData(), NULL, NULL, NULL);
        if (res) {
            QMessageBox msg(QMessageBox::Critical,"Save playlist as...", "Saving playlist failed.");
            msg.exec();
        }
        DBAPI->plt_unref(plt);
    }*/
}

void DefaultActions::on_actionAddFolder_triggered() {
    DBFileDialog *fd = new DBFileDialog(this, DBFileDialog::ADD_FOLDERS);
    fd->open();
    //QFileDialog fileDialog(this,
    //                        tr("Add folder(s) to playlist..."));
                            //QString(),
                            //QFileDialog::Directory,
                            //QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    //QStringList fileNames = fileDialog.exec();
    //if (fileNames.isEmpty())
    //    return;
    //foreach (QString localFile, fileNames)
    //    api->addTracksByUrl(localFile, DBAPI->pl_getcount(PL_MAIN) - 1);
}

void DefaultActions::on_actionAddFiles_triggered(){
    DBFileDialog *fd = new DBFileDialog(this, DBFileDialog::ADD_FILES);
    fd->open();
                            /*
                            tr("Add file(s) to playlist..."),
                            QStringList(),
                            QFileDialog::ExistingFiles,
                            QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        api->addTracksByUrl(localFile, DBAPI->pl_getcount(PL_MAIN) - 1);*/
}

void DefaultActions::on_actionAddAudioCD_triggered() {
    api->addTracksByUrl(QUrl("all.cda"));
}

void DefaultActions::on_actionAddURL_triggered() {
    api->addTracksByUrl(QUrl::fromUserInput(QInputDialog::getText(w, tr("Enter URL..."), tr("URL: "), QLineEdit::Normal)), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void DefaultActions::on_actionClearAll_triggered(){
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    if(DBAPI->plt_get_item_count(plt,PL_MAIN)) {
        if(QMessageBox::question(w,tr("Clear Playlist").append('?'),tr("Clear Playlist").append('?')) == QMessageBox::Yes) {
            api->clearPlaylist(DBAPI->plt_get_curr_idx());
        }
    }
    DBAPI->plt_unref(plt);
}

void DefaultActions::on_actionSelectAll_triggered() {
    DBAPI->pl_select_all();
    emit api->selectionChanged();
}

void DefaultActions::on_actionDeselectAll_triggered() {
    int count = DBAPI->pl_getcount(PL_MAIN);
    for (int i = 0; i < count; i++) {
        DB_playItem_t *it = DBAPI->pl_get_for_idx(i);
        DBAPI->pl_set_selected(it, false);
        DBAPI->pl_item_unref(it);
    }
    emit api->selectionChanged();
}

void DefaultActions::on_actionInvert_selection_triggered() {
    int count = DBAPI->pl_getcount(PL_MAIN);
    for (int i = 0; i < count; i++) {
        DB_playItem_t *it = DBAPI->pl_get_for_idx(i);
        DBAPI->pl_set_selected(it, !DBAPI->pl_is_selected(it));
        DBAPI->pl_item_unref(it);
    }
    emit api->selectionChanged();
}

void DefaultActions::on_actionSelectionRemove_triggered() {
    playItemList list;
    int count = DBAPI->pl_getcount(PL_MAIN);
    for (int i = 0; i < count; i++) {
        DB_playItem_t *it = DBAPI->pl_get_for_idx(i);
        if (DBAPI->pl_is_selected(it)) {
            list.append(it);
        }
    }
    if (list.count()) {
        api->removeTracks(list);
    }
}

void DefaultActions::on_actionSelectionCrop_triggered() {
    playItemList list;
    int count = DBAPI->pl_getcount(PL_MAIN);
    for (int i = 0; i < count; i++) {
        DB_playItem_t *it = DBAPI->pl_get_for_idx(i);
        if (!DBAPI->pl_is_selected(it)) {
            list.append(it);
        }
    }
    if (list.count()) {
        api->removeTracks(list);
    }
}

void DefaultActions::sortPlaylist(const char *format, bool ascending) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    DBAPI->plt_sort_v2(plt, PL_MAIN, -1, format, ascending);
    emit api->playlistContentChanged(plt);
    DBAPI->plt_unref(plt);
}

void DefaultActions::on_actionSortTitle_triggered() {
    static bool direction = false;
    sortPlaylist("%title%", direction = !direction);
}

void DefaultActions::on_actionSortTrackNumber_triggered() {
    static bool direction = false;
    sortPlaylist("%tracknumber%", direction = !direction);
}

void DefaultActions::on_actionSortArtist_triggered() {
    static bool direction = false;
    sortPlaylist("%artist%", direction = !direction);
}

void DefaultActions::on_actionSortAlbum_triggered() {
    static bool direction = false;
    sortPlaylist("%album%", direction = !direction);
}

void DefaultActions::on_actionSortDate_triggered() {
    static bool direction = false;
    sortPlaylist("%year%", direction = !direction);
}

void DefaultActions::on_actionSortRandom_triggered() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    DBAPI->plt_sort_v2(plt, PL_MAIN, -1, nullptr, DDB_SORT_RANDOM);
    emit api->playlistContentChanged(plt);
    DBAPI->plt_unref(plt);
}

void DefaultActions::on_actionSortCustom_triggered() {
    QString format = QInputDialog::getText(w,tr("Sort by").append("..."),tr("Format"));
    if (!format.isEmpty()) {
        QMessageBox msgbox(w);
        msgbox.setText(QString("%1 / %2?").arg(tr("Ascending"), tr("Descending")));
        msgbox.addButton(tr("Ascending"),QMessageBox::YesRole);
        msgbox.addButton(tr("Descending"),QMessageBox::NoRole);
        msgbox.exec();
        if (msgbox.result() == QMessageBox::AcceptRole) {
            sortPlaylist(format.toUtf8(),DDB_SORT_ASCENDING);
        }
        else if (msgbox.result() == QMessageBox::RejectRole) {
            sortPlaylist(format.toUtf8(),DDB_SORT_DESCENDING);
        }
    }
}

void DefaultActions::on_actionFind_triggered() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    if (plt) {
        DBAPI->plt_search_reset(plt);
        QDialog *dlg = new QDialog(w, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
        dlg->setWindowTitle(tr("Search"));
        dlg->setProperty("_internalNameWidget","search");
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        if (dlg->layout()) {
            delete dlg->layout();
        }
        QVBoxLayout *vbox = new QVBoxLayout(dlg);
        dlg->setLayout(vbox);
        QLineEdit *le = new QLineEdit(dlg);
        connect(le,SIGNAL(textEdited(QString)), this, SLOT(actionFind_searchBox_edited(QString)));
        vbox->addWidget(le);

        PlaylistModel *ptm = new PlaylistModel(plt,dlg,api);
        pv_search = new PlaylistView(dlg,api,ptm);
        pv_search->setDragEnabled(false);
        dlg->setMinimumSize(QSize(512,256));
        pv_search->pi_model->setIter(PL_SEARCH);
        vbox->addWidget(le);
        vbox->addWidget(pv_search);
        dlg->open();

        DBAPI->plt_unref(plt);
    }
}

void DefaultActions::actionFind_searchBox_edited(const QString str) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    if (plt && pv_search) {
        DBAPI->plt_search_process2(plt,str.toUtf8(), false);
        // HACK: full playlist reload
        pv_search->pi_model->setIter(PL_SEARCH);
        DBAPI->plt_unref(plt);
    }
}
