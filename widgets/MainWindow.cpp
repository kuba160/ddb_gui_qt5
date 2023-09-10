#include "MainWindow.h"

#include <QToolBar>
#include <QLabel>
#include <QMenu>

#include "DBActionMenu.h"

#define DBAPI (this->api)

MainWindow::MainWindow(QWidget *parent, DBApi *Api)
    : QMainWindow{parent},
      action_handlers(this, Api),
      plugins(this, Api)
{
    setObjectName("MainWindow");
    api = Api;

    bool rebuild_layout = DBAPI->conf.get("General", "RebuildLayout", true).toBool();
    if (rebuild_layout) {
        plugins.loadNewInstance(this, "playbackButtons", "Qt Widgets");
        plugins.loadNewInstance(this, "seekSlider", "Qt Widgets");
        plugins.loadNewInstance(this, "volumeSlider", "Qt Widgets");
        addToolBarBreak();
        plugins.loadNewInstance(this, "playbackButtons", "Qt Quick");
        plugins.loadNewInstance(this, "seekSlider", "Qt Quick");
        plugins.loadNewInstance(this, "volumeSlider", "Qt Quick");
        addToolBarBreak();
        plugins.loadNewInstance(this, "tabBar", "Qt Quick");
        addToolBarBreak();
        plugins.loadNewInstance(this, "tabBar", "Qt Widgets");
        //plugins.loadNewInstance(this, "playlist", "Qt Quick");
        plugins.loadNewInstance(this, "coverArt", "Qt Quick");
        plugins.loadNewInstance(this, "equalizer", "Qt Quick");
        plugins.loadNewInstance(this, "statusBar", "Qt Widgets");
        plugins.loadNewInstance(this, "playlist", "Qt Widgets");
        plugins.loadNewInstance(this, "actionsTree", "Qt Widgets");
        plugins.loadNewInstance(this, "queueManager", "Qt Widgets");

        //Api->conf.set("General", "RebuildLayout", true);
    }

    setCentralWidget(new QWidget());
    centralWidget()->setVisible(false);
    setDockNestingEnabled(true);

    QByteArray arr = DBAPI->conf.get("MainWindow", "state", QVariant()).toByteArray();
    if (!arr.isEmpty())
        restoreState(arr);
    arr = DBAPI->conf.get("MainWindow", "geometry", QVariant()).toByteArray();
    if (!arr.isEmpty())
        restoreGeometry(arr);
    //new QLabel

    // todo signal
    QString title = DBAPI->conf.get("MainWindow","titlebar_playing_tf", "DeaDBeeF - %artist% - %title%").toString();
    QString title_stopped = DBAPI->conf.get("MainWindow","titlebar_stopped_tf", "DeaDBeeF").toString();
    setWindowTitle(DBAPI->playback.tf_current(DBAPI->playback.getStopped() ? title_stopped : title));
    connect(&api->playback, &PlaybackControl::currentTrackChanged, this, [this] {
        QString title = DBAPI->conf.get("MainWindow","titlebar_playing_tf", "DeaDBeeF - %artist% - %title%").toString();
        QString title_stopped = DBAPI->conf.get("MainWindow","titlebar_stopped_tf", "DeaDBeeF").toString();
        setWindowTitle(DBAPI->playback.tf_current(DBAPI->playback.getStopped() ? title_stopped : title));
    });


    QMenuBar *menubar = buildMenuBar(this, Api);
    if (menubar) {
        setMenuBar(menubar);
    }


    // debug
    PlayItemIterator pit = PlayItemIterator();
    QJsonArray a = DBAPI->actions.parsePrototype(DBAction::ACTION_LOC_MENUBAR, pit);
    qDebug() << a;// QJsonDocument(a).toJson(QJsonDocument::Compact);
}

MainWindow::~MainWindow() {

}

void MainWindow::closeEvent(QCloseEvent *event) {
    api->conf.set("MainWindow", "geometry", saveGeometry());
    api->conf.set("MainWindow", "state", saveState());
    QMainWindow::closeEvent(event);
}

QMenu* MainWindow::createPopupMenu() {
    QMenu* menu = QMainWindow::createPopupMenu();
    QMenu *add = new QMenu("Add...", menu);
    QAbstractItemModel *model = plugins.loader.pluginLibraryModel();
    for (int i = 0; i < model->rowCount(); i++) {
        QVariant name = model->data(model->index(i, 0));
        add->addAction(name.toString());
    }

    menu->addMenu(add);
    return menu;
}
