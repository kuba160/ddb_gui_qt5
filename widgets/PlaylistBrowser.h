#ifndef PLAYLISTBROWSER_H
#define PLAYLISTBROWSER_H

#include <QWidget>
#include <QListWidget>
#include <QMouseEvent>
#include "DBApi.h"
#include "../PlaylistBrowserModel.h"

class PlaylistBrowser : public QListView, public DBWidget
{
    Q_OBJECT
public:
    PlaylistBrowser(QWidget *parent = nullptr, DBApi *Api = nullptr);
    ~PlaylistBrowser();

    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

    PlaylistBrowserModel *pbm;

protected slots:
    void mousePressEvent(QMouseEvent *ev);
    void onCurrentChanged(const QModelIndex &to, const QModelIndex &from);
    // select playlist in this widget
    void selectPlaylist(int);
    //
};

#endif // PLAYLISTBROWSER_H
