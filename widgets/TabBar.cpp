#include "TabBar.h"
#include "QtGui.h"

TabBar::TabBar(QWidget *parent, DBApi *Api) : QTabBar(parent), DBWidget(parent, Api) {
    setMovable(true);
    setTabsClosable(false);
    setSelectionBehaviorOnRemove(SelectLeftTab);

    // restore
    int cnt = api->getPlaylistCount();
    for (int i = 0; i < cnt; i++) {
        addTab(api->playlistNameByIdx(i));
    }
    setCurrentIndex(DBAPI->plt_get_curr_idx());

    connect(this, SIGNAL(tabMoved(int,int)), SLOT(onTabMoved(int,int)));
    connect(this, SIGNAL(currentChanged(int)), api, SLOT(changePlaylist(int)));
    connect(api, SIGNAL(playlistChanged(int)), this, SLOT(setCurrentIndex(int)));

    connect(api, SIGNAL(playlistMoved(int,int)), this, SLOT(onPlaylistMoved(int,int)));
    connect(api, SIGNAL(playlistRenamed(int)), this, SLOT(onPlaylistRenamed(int)));
    connect(api, SIGNAL(playlistCreated()), this, SLOT(onPlaylistCreated()));
    connect(api, SIGNAL(playlistRemoved(int)), this, SLOT(onPlaylistRemoved(int)));
}

QWidget *TabBar::constructor(QWidget *parent, DBApi *Api) {
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

QSize TabBar::tabSizeHint(int index) const {
    return QTabBar::tabSizeHint(index);
}

void TabBar::showTabContextMenu(int index, QPoint pos) {
    api->playlistContextMenu(this,mapToGlobal(pos),index);
}

void TabBar::onTabMoved(int to, int from) {
    api->movePlaylist(from, to);
}

void TabBar::onPlaylistMoved(int plt, int before) {
    Q_UNUSED(plt)
    Q_UNUSED(before)
    for (unsigned long i = 0; i < api->getPlaylistCount(); i++) {
        setTabText(i, api->playlistNameByIdx(i));
    }
    setCurrentIndex(DBAPI->plt_get_curr_idx());
}

void TabBar::onPlaylistRenamed(int tab) {
    setTabText(tab,api->playlistNameByIdx(tab));
}

void TabBar::onPlaylistCreated() {
    addTab("");
    for (unsigned long i = 0; i < api->getPlaylistCount(); i++) {
        setTabText(i, api->playlistNameByIdx(i));
    }
    setCurrentIndex(DBAPI->plt_get_curr_idx());
}

void TabBar::onPlaylistRemoved(int plt) {
    removeTab(plt);
}
