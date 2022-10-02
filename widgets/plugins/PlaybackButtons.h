#ifndef PLAYBACKBUTTONS_H
#define PLAYBACKBUTTONS_H

#include <QToolBar>
#include <QStyle>
#include <QSize>
#include <dbapi/DBApi.h>

class PlaybackButtons : public QWidget {
    Q_OBJECT
public:
    PlaybackButtons(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QObject *constructor(QWidget *parent = nullptr, DBApi *Api =nullptr);

private:
    //QList<QAction *> action_list;
    //QMenu menu;
public slots:
    //void customContextMenuRequested(QPoint pos);
};

#endif // PLAYBACKBUTTONS_H
