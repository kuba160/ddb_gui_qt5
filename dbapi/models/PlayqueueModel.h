#ifndef PLAYQUEUEMODEL_H
#define PLAYQUEUEMODEL_H

#include <QAbstractItemModel>
#include <QMimeData>

#include "PlayItemModel.h"


class PlayqueueModel : public PlayItemModel {
    Q_OBJECT
public:
    PlayqueueModel(QObject *parent, PlaylistManager *Manager);
    ~PlayqueueModel();

    // PlayItem implementation
    playItemList tracks(const QList<int> &tracks) const override;
    void insertTracks(playItemList *l, int after) override;
    void moveIndexes(QList<int> indices, int after) override;
    void removeIndexes(QList<int> indices) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    ddb_playlist_t *plt;

    Q_PROPERTY(int length READ getLength NOTIFY lengthChanged)

    int getLength();
signals:
    void lengthChanged();
protected slots:
    void onQueueChanged();
};

#endif // PLAYQUEUEMODEL_H
