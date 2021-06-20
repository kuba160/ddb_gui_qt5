#ifndef TABBAR_H
#define TABBAR_H

#include <QTabBar>
#include "DBApi.h"

#define DRAG_DELAY 5

class TabBar: public QTabBar, public DBWidget {
    Q_OBJECT
public:
    TabBar(QWidget *parent = nullptr, DBApi *Api = nullptr);

    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event);
    QSize tabSizeHint(int index) const;
    

private slots:
    void showTabContextMenu(int, QPoint);
    void onPlaylistRenamed(int);
    void onTabMoved(int,int);
    void onPlaylistMoved(int, int);
    void onPlaylistCreated();
    void onPlaylistRemoved(int);

};

#endif

