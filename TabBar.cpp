#include "TabBar.h"

#include <QMouseEvent>
#include <QInputDialog>

#include "QtGui.h"

#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

TabBar::TabBar(QWidget *parent) : QTabBar(parent), tabContextMenu(this) {
    configure();
    fillTabs();
    buildTabContextMenu();
    selectLastTab();
    createConnections();
}

TabBar::~TabBar() {
    delete delPlaylist;
    delete renPlaylist;
    delete addPlaylist;
}

QWidget *TabBar::constructor(QWidget *parent, DBApi *Api) {
    Q_UNUSED(Api);
    QWidget *widget = new TabBar(parent);
    return widget;
}

QDockWidget *TabBar::constructorDockable(QWidget *parent, DBApi *Api) {
    Q_UNUSED(Api);
    QWidget *widget = new TabBar(parent);
    QDockWidget *dock = new QDockWidget(parent);
    dock->setWindowTitle(QString("Tab Bar"));
    dock->setWidget(widget);
    return dock;
}

void TabBar::createConnections() {
    connect(this, SIGNAL(tabCloseRequested(int)), SLOT(closeTab(int)));
    connect(this, SIGNAL(tabContextMenuRequested(int, QPoint)), SLOT(showTabContextMenu(int, QPoint)));
    connect(this, SIGNAL(tabMoved(int,int)), SLOT(moveTab(int,int)));
}

void TabBar::fillTabs() {
    int cnt = DBAPI->plt_get_count();
    char title[100];
    for (int i = 0; i < cnt; i++) {
        DBAPI->pl_lock();
        DBAPI->plt_get_title(DBAPI->plt_get_for_idx(i), title, sizeof(title));
        DBAPI->pl_unlock();
        addTab(QString::fromUtf8(title));
        strcpy(title, "");
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
    if (count() == 1) {
        //setTabsClosable(false);
        delPlaylist->setEnabled(false);
    }
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
    if (!(event->orientation() == Qt::Horizontal)) {
        if (receivers(SIGNAL(wheelDelta(int)))) {
            emit(wheelDelta(event->delta()));
            return;
        }
        int lastIndex = count() - 1;
        int targetIndex = -1;
        bool forward = event->delta() < 0;
        if (forward && lastIndex == currentIndex()) {
            event->ignore();
            return;
            targetIndex = 0;
        }
        else
            if (!forward && 0 == currentIndex()) {
                targetIndex = lastIndex;
                event->ignore();
                return;
            }
        setCurrentIndex(targetIndex);
        if (targetIndex != currentIndex() || !isTabEnabled(targetIndex))
            QTabBar::wheelEvent(event);
        event->accept();
        emit tabSelected(currentIndex());
    } else
        event->ignore();
}

int TabBar::selectTab(const QPoint &pos) const {
    const int tabCount = count();
    for (int i = 0; i < tabCount; ++i)
        if (tabRect(i).contains(pos))
            return i;
    return -1;
}

void TabBar::buildTabContextMenu() {
    renPlaylist = new QAction(tr("Rename playlist"), &tabContextMenu);
    connect(renPlaylist, SIGNAL(triggered()), this, SLOT(renamePlaylist()));
    addPlaylist = new QAction(tr("Add new playlist"), &tabContextMenu);
    connect(addPlaylist, SIGNAL(triggered()), this, SLOT(newPlaylist()));
    delPlaylist = new QAction(tr("Remove playlist"), &tabContextMenu);
    connect(delPlaylist, SIGNAL(triggered()), this, SLOT(closeTab()));
    
    tabContextMenu.addAction(renPlaylist);
    tabContextMenu.addAction(addPlaylist);
    tabContextMenu.addAction(delPlaylist);

    tabContextMenu.addSeparator();

    QMenu *positionMenu = tabContextMenu.addMenu(tr("Tabbar position"));
    top = positionMenu->addAction(tr("Top"));
    top->setCheckable(true);
    connect(top, SIGNAL(toggled(bool)), SLOT(setTopPosition()));
    bottom = positionMenu->addAction(tr("Bottom"));
    bottom->setCheckable(true);
    connect(bottom, SIGNAL(toggled(bool)), SLOT(setBottomPosition()));
    left = positionMenu->addAction(tr("Left"));
    left->setCheckable(true);
    connect(left, SIGNAL(toggled(bool)), SLOT(setLeftPosition()));
    right = positionMenu->addAction(tr("Right"));
    right->setCheckable(true);
    connect(right, SIGNAL(toggled(bool)), SLOT(setRightPosition()));
    QActionGroup *positionGroup = new QActionGroup(positionMenu);
    positionGroup->addAction(top);
    positionGroup->addAction(bottom);
    positionGroup->addAction(left);
    positionGroup->addAction(right);
    positionGroup->setExclusive(true);
    top->setChecked(true);
}

void TabBar::showTabContextMenu(int index, QPoint globalPos) {
    indexForAction = index;
    tabContextMenu.move(globalPos);
    tabContextMenu.show();
}

void TabBar::moveTab(int to, int from) {
    DBAPI->plt_move(from, to);
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
                delPlaylist->setEnabled(true);
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
        delPlaylist->setEnabled(false);
    }
    removeTab(index);
    emit tabClosed(index);
}

void TabBar::closeTab() {
    closeTab(indexForAction);
}

void TabBar::renamePlaylist() {
    bool ok;
    QString newName = QInputDialog::getText(this, tr("Choose new name"), tr("Enter new playlist name: "), QLineEdit::Normal, tabText(indexForAction), &ok);
    if (ok && !newName.isEmpty()) {
        setTabText(indexForAction, newName);
        emit tabRenamed(indexForAction, newName);
    }
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

void TabBar::setShape(QTabBar::Shape shape) {
    switch (shape) {
        case QTabBar::RoundedNorth:
            top->setChecked(true);
            break;
        case QTabBar::RoundedSouth:
            bottom->setChecked(true);
            break;
        case QTabBar::RoundedWest:
            left->setChecked(true);
            break;
        case QTabBar::RoundedEast:
            right->setChecked(true);
            break;
        default:
            break;
    }
    QTabBar::setShape(shape);
}

