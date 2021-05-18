#ifndef PLAYLISTBROWSER_H
#define PLAYLISTBROWSER_H

#include <QWidget>
#include <QListWidget>
#include <QMouseEvent>
#include "DBApi.h"

class PlaylistBrowser : public QListWidget, public DBWidget
{
    Q_OBJECT
public:
    PlaylistBrowser(QWidget *parent = nullptr, DBApi *Api = nullptr);

    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

private slots:
    void onItemClicked(QListWidgetItem *);

private:
    void dropEvent(QDropEvent *);

    // used to detect if move event comes from this widget
    int our_pl = -1;
    int our_before = -1;

protected slots:
    void mousePressEvent(QMouseEvent *ev);

signals:
    // playlist got selected by this widget
    void playlistSelected(int);

private slots:
    // select playlist in this widget
    void selectPlaylist(int);
    // move playlist pl to before
    void playlistOrderChanged(int pl, int before);
    //
    void onPlaylistRenamed(int);
    void onPlaylistRemoved(int);
    void onPlaylistCreated();
};

#endif // PLAYLISTBROWSER_H
