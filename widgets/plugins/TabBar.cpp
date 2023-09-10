#include "TabBar.h"

#include <QMouseEvent>

#define DBAPI (this->api)

TabBar::TabBar(QWidget *parent, DBApi *Api) : QTabBar(parent) {
    api = Api;
    setMovable(true);
    setTabsClosable(false);
    setSelectionBehaviorOnRemove(SelectLeftTab);

    // restore
    list = api->playlist.getList();
    int cnt = list->rowCount();
    for (int i = 0; i < cnt; i++) {
        addTab(list->data(list->index(i,0)).toString());
    }

    // index
    setCurrentIndex(api->playlist.getCurrentPlaylistIdx());
    connect(&DBAPI->playlist, &PlaylistManager::currentPlaylistChanged, this,
            [this]() {setCurrentIndex(DBAPI->playlist.getCurrentPlaylistIdx());});
    connect(this, &QTabBar::currentChanged, &api->playlist, &PlaylistManager::setCurrentPlaylistIdx);

    // move
    connect(list, &QAbstractItemModel::rowsMoved, this,
            [this](const QModelIndex &, int start, int end, const QModelIndex &, int row) {
        while (start < end) {
            this->moveTab(start++, row++);
        }
    });
    connect(this, &QTabBar::tabMoved, this, [this](int from, int to) {
        list->moveRows({}, qMax(from,to), 1, {}, qMin(from,to));
    });

    // rename
    connect(list, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight,
            const QList<int> &roles = QList<int>()) {
        for (int i = topLeft.row(); i <= bottomRight.row(); i++) {
            this->setTabText(i, list->data(list->index(i,0)).toString());
        }
    });

    // delete
    connect(list, &QAbstractItemModel::rowsRemoved, this,
            [this](const QModelIndex &, int first, int last) {
        for (int i = first; i <= last; i++) {
            this->removeTab(i);
        }
    });

    // insert
    connect(list, &QAbstractItemModel::rowsInserted, this,
            [this](const QModelIndex &, int first, int last) {
        for (int i = first; i <= last; i++) {
            this->insertTab(i, list->data(list->index(i,0)).toString());
        }
    });
}

QObject *TabBar::constructor(QWidget *parent, DBApi *Api) {
    if (!parent) {
        QObject *info = new QObject(nullptr);
        info->setProperty("friendlyName", info->tr("Tab Strip"));
        info->setProperty("internalName", "tabBar");
        info->setProperty("widgetType", "toolbar");
        info->setProperty("widgetStyle", "Qt Widgets");
        return info;
    }
    QWidget *widget = new TabBar(parent, Api);
    return widget;
}

void TabBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        showTabContextMenu(tabAt(event->pos()), event->pos());
        event->accept();
        return;
    }
    QTabBar::mousePressEvent(event);
}

void TabBar::showTabContextMenu(int index, QPoint pos) {

   // api->playlistContextMenu(this,mapToGlobal(pos),index);
}
