#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QMimeData>

#include "DBApi.h"
#include "PlayItemModel.h"

class PlaylistModel : public PlayItemTableModel {
    Q_OBJECT
public:
    PlaylistModel(QObject *parent = nullptr, DBApi *api = nullptr);
    PlaylistModel(ddb_playlist_t *plt, QObject *parent = nullptr, DBApi *Api = nullptr);
    ~PlaylistModel();

    // playlist manipulation
    void setPlaylist(ddb_playlist_t *plt);

    // PlayItemModel implementation
    playItemList tracks(const QList<int> &tracks) const override;
    void insertTracks(playItemList *l, int after) override;
    void moveIndexes(QList<int> indices, int after) override;
    void removeIndexes(QList<int> indices) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    ddb_playlist_t *plt = nullptr;

private slots:
    void onPlaylistContentChanged(ddb_playlist_t *plt);
};


#endif // PLAYLISTMODEL_H
