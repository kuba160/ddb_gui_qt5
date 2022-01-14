#ifndef PLAYQUEUEMODEL_H
#define PLAYQUEUEMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>

#include "DBApi.h"
#include "PlayItemModel.h"


class PlayqueueModel : public PlayItemTableModel {
    Q_OBJECT
public:
    PlayqueueModel(QObject *parent = nullptr, DBApi *api = nullptr);
    ~PlayqueueModel();

    // PlayItem implementation
    playItemList tracks(const QList<int> &tracks) const override;
    void insertTracks(playItemList *l, int after) override;
    void moveIndexes(QList<int> indices, int after) override;
    void removeIndexes(QList<int> indices) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    ddb_playlist_t *plt;
protected slots:
    void onQueueChanged();
};

#endif // PLAYQUEUEMODEL_H
