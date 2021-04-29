#include "TabBar.h"

#include <QMouseEvent>
#include <QInputDialog>

#include "QtGui.h"

#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

TabBar::TabBar(QWidget *parent, DBApi *Api) : QTabBar(parent), DBWidget(parent, Api), tabContextMenu(this)  {
    configure();
    fillTabs();
    selectLastTab();
    createConnections();

    connect(api, SIGNAL(playlistMoved(int,int)), this, SLOT(playlistOrderChanged(int,int)));
    connect(this, SIGNAL(tabRenamed(int,const QString*)), api, SLOT(renamePlaylist(int,const QString*)));
    connect(api, SIGNAL(playlistRenamed(int)), this, SLOT(onPlaylistRenamed(int)));
}

QWidget *TabBar::constructor(QWidget *parent, DBApi *Api) {
    QWidget *widget = new TabBar(parent, Api);
    return widget;
}

void TabBar::createConnections() {
    connect(this, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
    connect(this, SIGNAL(tabContextMenuRequested(int,QPoint)), SLOT(showTabContextMenu(int,QPoint)));
    connect(this, SIGNAL(tabMoved(int,int)), SLOT(moveTab(int,int)));
    connect(this, SIGNAL(tabSelected(int)), api, SLOT(changePlaylist(int)));
    connect(api, SIGNAL(playlistChanged(int)), this, SLOT(setCurrentIndex(int)));
}

void TabBar::fillTabs() {
    int cnt = api->getPlaylistCount();
    for (int i = 0; i < cnt; i++) {
        addTab(api->playlistNameByIdx(i));
    }
}

void TabBar::configure() {
    setAcceptDrops(true);
    setMouseTracking(true);
    setMovable(true);
    //setTabsClosable(true);
    setTabsClosable(false);
    setSelectionBehaviorOnRemove(SelectLeftTab);
}

void TabBar::selectLastTab() {
    setCurrentIndex(DBAPI->plt_get_curr_idx());
}

void TabBar::mouseDoubleClickEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton)
        return;

    int tab = selectTab(event->pos());

    if (tab == -1)
        newPlaylist();
    else
        emit tabDoubleClicked(tab);

    QTabBar::mouseDoubleClickEvent(event);
}

void TabBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int tab = selectTab(event->pos());
        if (tab != -1)
            emit tabSelected(tab);
    } else if (event->button() == Qt::RightButton) {
        int tab = selectTab(event->pos());
        if (tab != -1)
            emit tabContextMenuRequested(tab, mapToGlobal(event->pos()));
        else
            emit emptyAreaContextMenuRequested(mapToGlobal(event->pos()));
        return;
    }

    QTabBar::mousePressEvent(event);
}

QSize TabBar::tabSizeHint(int index) const {
    return QTabBar::tabSizeHint(index);
}

void TabBar::wheelEvent(QWheelEvent *event) {
    // TODO
    QTabBar::wheelEvent(event);
    emit tabSelected(currentIndex());
}

int TabBar::selectTab(const QPoint &pos) const {
    const int tabCount = count();
    for (int i = 0; i < tabCount; ++i)
        if (tabRect(i).contains(pos))
            return i;
    return -1;
}


void TabBar::showTabContextMenu(int index, QPoint pos) {
    indexForAction = index;
    api->playlistContextMenu(this,mapFromGlobal(pos),index);
}

void TabBar::moveTab(int to, int from) {
    if (our_external) {
        our_external = false;
        return;
    }
    our_pl = from;
    our_before = to;
    api->movePlaylist(from, to);
}

void TabBar::playlistOrderChanged(int plt, int before) {
    if (plt == our_pl && before == our_before) {
        // Ignore order change done by this widget
        our_pl = -1;
        our_before = -1;
        return;
    }
    our_external = true;
    QTabBar::moveTab(plt, before);
}

void TabBar::onPlaylistRenamed(int tab) {
    setTabText(tab,api->playlistNameByIdx(tab));
}

void TabBar::newPlaylist() {
    int cnt = DBAPI->plt_get_count();
    int i;
    int idx = 0;
    for (;;) {
        QString name = "";
        if (!idx)
            name = tr("New Playlist");
        else
            name = tr("New Playlist (%1)").arg(idx);
        DBAPI->pl_lock();
        for (i = 0; i < cnt; i++) {
            char t[100];
            DBAPI->plt_get_title(DBAPI->plt_get_for_idx(i), t, sizeof(t));
            if (!strcasecmp (t, name.toUtf8().constData()))
                break;
        }
        DBAPI->pl_unlock();
        if (i == cnt) {
            DBAPI->plt_add(cnt, name.toUtf8().constData());
            if (count() == 1) {
                //setTabsClosable(true);
            }
            insertTab(cnt, name);
            setCurrentIndex(cnt);
            emit tabSelected(cnt);
            return;
        }
        idx++;
    }
}

void TabBar::closeTab(int index) {
    if (count() == 2) {
        setTabsClosable(false);
    }
    removeTab(index);
    emit tabClosed(index);
}

void TabBar::closeTab() {
    closeTab(indexForAction);
}

void TabBar::renamePlaylist() {
    api->renamePlaylist(indexForAction);
}

void TabBar::setBottomPosition() {
    emit changeTabPosition(TabBar::Bottom);
}

void TabBar::setLeftPosition() {
    emit changeTabPosition(TabBar::Left);
}

void TabBar::setRightPosition() {
    emit changeTabPosition(TabBar::Right);
}

void TabBar::setTopPosition() {
    emit changeTabPosition(TabBar::Top);
}
