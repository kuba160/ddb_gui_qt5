#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "DBApi.h"
#include "PlaylistView.h"
#include "PlaylistModel.h"

class Playlist : public PlaylistView {
    Q_OBJECT

public:
    Playlist(QWidget *parent = nullptr, DBApi *Api = nullptr, PlaylistModel *ptm = nullptr);
    ~Playlist();
    static QWidget *constructor(QWidget *parent, DBApi *Api);

protected:
    PlaylistModel *ptm = nullptr;

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
