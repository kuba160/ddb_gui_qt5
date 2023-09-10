#ifndef DBACTIONMENU_H
#define DBACTIONMENU_H

#include <QMenu>
#include <QMenuBar>
#include <QObject>

#include <dbapi/DBApi.h>
#include <dbapi/Actions.h>

//class DBMenuBuilder : public QObject {
//    Q_OBJECT
//public:
//    static QMenu
//};

//class DBActionMenu : public QMenu {
//    PlayItemIterator *context;
//    DBApi *dbapi;
//public:
//    DBActionMenu(QWidget *parent, QJsonObject menu_def, DBApi *Api);
//    ~DBActionMenu();

//signals:
//    void actionTriggered(QString);
//};

//class DBActionMenuBar : public QMenuBar{
//    PlayItemIterator *context;
//public:
//    DBActionMenuBar(QWidget *parent, QJsonObject menu_def, DBApi *Api);

//public slots:
//    void onActionTriggered(QAction *);

//};

QMenuBar* buildMenuBar(QWidget *parent, DBApi *Api);
QMenu* buildTrackContextMenu(QWidget *parent, DBApi *Api, PlayItemIterator pit);

#endif // DBACTIONMENU_H
