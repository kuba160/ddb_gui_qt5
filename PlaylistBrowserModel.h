#ifndef PLAYLISTBROWSERMODEL_H
#define PLAYLISTBROWSERMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <DBApi.h>

class PlaylistBrowserModel : public QAbstractListModel, public DBWidget {
    Q_OBJECT
public:
    PlaylistBrowserModel(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~PlaylistBrowserModel();

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    Qt::DropActions supportedDropActions() const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
private slots:
    void onPlaylistMoved(int, int);
    void onPlaylistCreated();
    void onPlaylistRenamed(int);
    void onPlaylistRemoved(int);
};

#endif // PLAYLISTBROWSERMODEL_H
