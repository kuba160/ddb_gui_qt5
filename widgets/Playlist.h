#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "DBApi.h"
#include "PlaylistView.h"

class Playlist : public PlaylistView {
    Q_OBJECT

public:
    Playlist(QWidget *parent = nullptr, DBApi *Api = nullptr);
    ~Playlist();
    static QWidget *constructor(QWidget *parent, DBApi *Api);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
private slots:
    void onSelectionChanged();
    void onPlaylistChanged();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void trackDoubleClicked(QModelIndex index);
    void onTrackChanged();
public slots:
    void jumpToCurrentTrack();
};

#endif // PLAYLIST_H
