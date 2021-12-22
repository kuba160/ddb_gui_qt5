#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QMimeData>

#include "DBApi.h"
#include "PlayItemModel.h"

class PlaylistModel : public PlayItemModel {
    Q_OBJECT
    
public:
    PlaylistModel(QObject *parent = nullptr, DBApi *Api = nullptr);
    PlaylistModel(ddb_playlist_t *plt, QObject *parent = nullptr, DBApi *Api = nullptr);
    ~PlaylistModel();

    // playlist manipulation
    void setPlaylist(ddb_playlist_t *plt);
    void insertTracks(playItemList *l, int after);
    void moveIndexes(QList<int> indices, int after);
    void removeIndexes(QList<int> indices);

    playItemList tracks(const QModelIndexList &tracks) const;
    playItemList tracks(const QList<int> &tracks) const;
    DB_playItem_t *track(const QModelIndex &track) const;

    // Qt Quick
    // Playlist property based on iter
    Q_PROPERTY(int plt_num READ getPltNum WRITE setPltNum NOTIFY pltNumChanged)
    void setPltNum(int plt);
    int getPltNum();
protected:
    int m_plt_num;
signals:
    void pltNumChanged();

protected:
    int rowCount(const QModelIndex &parent) const;
    int trackCount() const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
    ddb_playlist_t *plt = nullptr;

private slots:
    void onPlaylistContentChanged(ddb_playlist_t *plt);
};


#endif // PLAYLISTMODEL_H
