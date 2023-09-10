#ifndef PLAYLISTBROWSERMODEL_H
#define PLAYLISTBROWSERMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include "../PlaylistManager.h"

class PlaylistBrowserModel : public QAbstractListModel {
    Q_OBJECT
    PlaylistManager *manager;
public:
    PlaylistBrowserModel(QObject *parent, PlaylistManager *Manager);
    ~PlaylistBrowserModel();

    enum PlaylistsRoles {
        PlaylistNameRole = Qt::UserRole + 1,
        PlaylistItemsRole,
        PlaylistLengthRole,
        PlaylistIterator
    };
    QHash<int, QByteArray> roleNames() const override;


    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    /// manipulation
    // title change
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    // new playlist insert
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    // move playlist
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    // remowe
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Drag'n'drop
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

public slots:
    void refreshPlaylist();
private slots:
    void onPlaylistMoved(int, int);
    void onPlaylistCreated();
    void onPlaylistRenamed(int);
    void onPlaylistRemoved(int);
};

#endif // PLAYLISTBROWSERMODEL_H
