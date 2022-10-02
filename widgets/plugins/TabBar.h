#ifndef TABBAR_H
#define TABBAR_H

#include <QTabBar>
#include <dbapi/DBApi.h>

#define DRAG_DELAY 5

class TabBar: public QTabBar {
    DBApi *api;
    QAbstractItemModel *list;
    Q_OBJECT
public:
    TabBar(QWidget *parent = nullptr, DBApi *Api = nullptr);

    static QObject *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event);
    

private slots:
    void showTabContextMenu(int, QPoint);
};

#endif

