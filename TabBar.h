#ifndef TABBAR_H
#define TABBAR_H

#include <QTabBar>
#include <QMenu>
#include <QDockWidget>

#include "DBApi.h"

#define DRAG_DELAY 5

class TabBar: public QTabBar {
    Q_OBJECT
public:
    enum TabBarPosition {
        Top = 0,
        Bottom = 1,
        Left = 2,
        Right = 3,
    };
    TabBar(QWidget *parent = nullptr);
    ~TabBar();

    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QDockWidget *constructorDockable(QWidget *parent = nullptr, DBApi *Api = nullptr);
    int selectTab(const QPoint &position) const;
    void setShape(Shape shape);
    
private:
    QMenu tabContextMenu;
    QAction *delPlaylist;
    QAction *addPlaylist;
    QAction *renPlaylist;

    QAction *top;
    QAction *bottom;
    QAction *left;
    QAction *right;

    void buildTabContextMenu();
    void configure();
    void fillTabs();
    void selectLastTab();
    void createConnections();
    
    int indexForAction;

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    QSize tabSizeHint(int index) const;

public Q_SLOTS:
    void newPlaylist();
    

private Q_SLOTS:
    void moveTab(int, int);
    void showTabContextMenu(int, QPoint);
    void renamePlaylist();
    void closeTab();
    void closeTab(int);
    void setTopPosition();
    void setBottomPosition();
    void setRightPosition();
    void setLeftPosition();

Q_SIGNALS:
    void tabContextMenuRequested(int index, const QPoint &globalPos);
    void emptyAreaContextMenuRequested(const QPoint &globalPos);

    void tabDoubleClicked(int index);
    void mouseMiddleClick(int index);
    void wheelDelta(int);

    void tabClosed(int);
    void tabSelected(int);
    void tabRenamed(int, const QString &);

    void changeTabPosition(TabBar::TabBarPosition);
};

#endif

