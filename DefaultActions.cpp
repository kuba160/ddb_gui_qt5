#include "DefaultActions.h"
#include "ui_DefaultActions.h"
#include <QObject>
#include <QStyle>
#include "include/callbacks.h"

#include "AboutDialog.h"

#include "QtGui.h"


DefaultActions::DefaultActions(DBApi *Api, QWidget *parent) : QWidget(parent), DBWidget(parent,Api), ui(new Ui::DefaultActions) {
    ui->setupUi(this);

    // Icons
    ui->actionExit->setIcon(getStockIcon(this, "application-exit", QStyle::SP_DialogCloseButton));
    ui->actionClearAll->setIcon(getStockIcon(this, "edit-clear", QStyle::SP_TrashIcon));
    ui->actionAddFiles->setIcon(getStockIcon(this, "document-open", QStyle::SP_FileIcon));
    ui->actionAddFolder->setIcon(getStockIcon(this, "folder-m", QStyle::SP_DirIcon));
    ui->actionAddURL->setIcon(getStockIcon(this, "folder-remote", QStyle::SP_DriveNetIcon));
    ui->actionAddAudioCD->setIcon(getStockIcon(this, "media-optical-audio", QStyle::SP_DriveCDIcon));
    ui->actionNewPlaylist->setIcon(getStockIcon(this, "document-new", QStyle::SP_FileDialogNewFolder));
    ui->actionPreferences->setIcon(getStockIcon(this, "preferences-system", QStyle::SP_CustomBase));
    ui->actionAbout->setIcon(getStockIcon(this, "help-about", QStyle::SP_DialogHelpButton));
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
        // Restore
        shuffle[DBAPI->streamer_get_shuffle()]->setChecked(true);
        repeat[DBAPI->streamer_get_repeat()]->setChecked(true);
    }

    main_widgets = ui->menuView->addMenu(tr("Main Widget"));
    main_widgets_list = new QActionGroup(nullptr);
    new_plugins = ui->menuView->addMenu(QString("%1...") .arg(tr("Add")));
    remove_plugins = ui->menuView->addMenu(QString("%1...") .arg(tr("Remove")));
    //remove_plugins->menuAction()->setVisible(false);

    // New widget creation
    {
        // add alreavy loaded widgets
        unsigned int i = 0;
        QAction *a;

        for (i = 0; (a = pl->actionNewGet(i)); i++) {
            new_plugins->addAction(a);
        }
        // subscribe for future actions
        connect (pl, SIGNAL(actionPluginAddCreated(QAction *)), this, SLOT(onWidgetAddAction(QAction *)));
    }

    // Deletion of existent plugins
    {
        // subscribe for future actions
        connect (pl, SIGNAL(actionPluginRemoveCreated(QAction *)), this, SLOT(onWidgetRemoveAction(QAction *)));
    }

    if (pl->getTotalMainWidgets() <= 1) {
        main_widgets->menuAction()->setVisible(false);
    }

    // MainWidget selection
    connect (pl, SIGNAL(actionPluginMainWidgetCreated(QAction *)), this, SLOT(onMainWidgetAdded(QAction *)));

    //connect (this, SIGNAL(configLoaded()), pl, SLOT(updateActionChecks()));

    connect (this->ui->actionBlockToolbarChanges, SIGNAL(toggled(bool)), pl, SLOT(lockWidgets(bool)));

    //connect (this->ui->actionExit, SIGNAL(triggered()), pl, SLOT(actionChecksSave()));

    connect (pl, SIGNAL(actionToggleVisibleCreated(QAction *)), this, SLOT(onActionToggleCreated(QAction *)));


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

void DefaultActions::onWidgetAddAction(QAction *action) {
    new_plugins->addAction(action);
}

void DefaultActions::onWidgetRemoveAction(QAction *action) {
    remove_plugins->setVisible(true);
    remove_plugins->addAction(action);
}

void DefaultActions::onActionToggleCreated(QAction *action) {
    ui->menuView->addAction(action);
}


void DefaultActions::onMainWidgetAdded(QAction *action) {
    if (action == nullptr) {
        if (pl->getTotalMainWidgets() > 1) {
            main_widgets->menuAction()->setVisible(true);
        }
        else {
            main_widgets->menuAction()->setVisible(false);
        }
        return;
    }
    main_widgets_list->addAction(action);
    main_widgets->addAction(action);
    if (pl->getTotalMainWidgets() > 1) {
        main_widgets->menuAction()->setVisible(true);
    }
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
    qDebug() << "test";
}
