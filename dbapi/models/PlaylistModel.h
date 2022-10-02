#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>

#include "PlayItemModel.h"
#include <QMutex>

class PlaylistModel : public PlayItemModel {
    Q_OBJECT
public:
    PlaylistModel(QObject *parent, PlaylistManager *Manager, ddb_playlist_t *plt = nullptr);
    //PlaylistModel(ddb_playlist_t *plt, QObject *parent = nullptr, DBApi *Api = nullptr);
    ~PlaylistModel();

    // playlist manipulation
    Q_PROPERTY(ddb_playlist_t* plt READ getPlaylist WRITE setPlaylist NOTIFY playlistChanged)
    ddb_playlist_t* getPlaylist();
    void setPlaylist(ddb_playlist_t *plt);

    // PlayItemModel implementation
    playItemList tracks(const QList<int> &tracks) const override;
    void insertTracks(playItemList *l, int after) override;
    void moveIndexes(QList<int> indices, int after) override;
    void removeIndexes(QList<int> indices) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

signals:
    void playlistChanged(ddb_playlist_t *);

private:
    ddb_playlist_t *plt = nullptr;
    QMutex mut;

private slots:
    void onPlaylistContentChanged(ddb_playlist_t *plt);
};


#endif // PLAYLISTMODEL_H
