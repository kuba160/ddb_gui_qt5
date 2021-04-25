#ifndef PLAYQUEUEMODEL_H
#define PLAYQUEUEMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>

#include "DBApi.h"
#include "PlayItemModel.h"

class PlayqueueModel : public PlayItemModel {
    Q_OBJECT

public:
    PlayqueueModel(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~PlayqueueModel();

    // playlist manipulation
    void insertTracks(playItemList *l, int after);
    void moveIndexes(QList<int> indices, int after);
    void removeIndexes(QList<int> indices);

    playItemList tracks(const QModelIndexList &tracks) const;
    playItemList tracks(const QList<int> &tracks) const;
    DB_playItem_t *track(const QModelIndex &track) const;
protected:
    int rowCount(const QModelIndex &parent) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    ddb_playlist_t *plt;
protected slots:
    void onQueueChanged();
};

#endif // PLAYQUEUEMODEL_H
