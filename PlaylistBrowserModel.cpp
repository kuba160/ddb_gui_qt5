#include "PlaylistBrowserModel.h"

PlaylistBrowserModel::PlaylistBrowserModel(QObject *parent, DBApi *Api) : QAbstractListModel(parent), DBWidget(nullptr, Api) {

    connect(api, SIGNAL(playlistMoved(int,int)), this, SLOT(onPlaylistMoved(int,int)));
    connect(api, SIGNAL(playlistCreated()), this, SLOT(onPlaylistCreated()));
    connect(api, SIGNAL(playlistRenamed(int)), this, SLOT(onPlaylistRenamed(int)));
    connect(api, SIGNAL(playlistRemoved(int)), this, SLOT(onPlaylistRemoved(int)));
}

PlaylistBrowserModel::~PlaylistBrowserModel() {

}

int PlaylistBrowserModel::rowCount(const QModelIndex &parent) const {
    return DBAPI->plt_get_count();
}

QVariant PlaylistBrowserModel::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            if (index.row() <= rowCount(index)) {
                ddb_playlist_t *plt = DBAPI->plt_get_for_idx(index.row());
                if (plt) {
                    char buf[512];
                    DBAPI->plt_get_title(plt, buf, 512);
                    DBAPI->plt_unref(plt);
                    return QVariant(QString(buf));
                }
            }
        }
    }
    return QVariant();
}

void PlaylistBrowserModel::onPlaylistMoved(int plt, int before) {
    beginResetModel();
    endResetModel();
    return; // TODO
    beginMoveRows(QModelIndex(),plt,plt, QModelIndex(), before);
    endMoveRows();
}

void PlaylistBrowserModel::onPlaylistCreated() {
    // unsure which row playlist is added, update all
    beginResetModel();

    endResetModel();
    //beginInsertRows()
}

void PlaylistBrowserModel::onPlaylistRenamed(int plt) {
    emit dataChanged(index(plt),index(plt));
}

void PlaylistBrowserModel::onPlaylistRemoved(int plt) {
    beginRemoveRows(QModelIndex(),plt,plt);
    endRemoveRows();
}

Qt::ItemFlags PlaylistBrowserModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}


Qt::DropActions PlaylistBrowserModel::supportedDropActions() const {
    return Qt::MoveAction;
}

bool PlaylistBrowserModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    // put on end if not valid
    if (row == -1) {
        row = rowCount(QModelIndex());
    }
    while (!stream.atEnd()) {
        int nrow, ncol;
        QMap<int,  QVariant> roleDataMap;
        stream >> nrow >> ncol >> roleDataMap;
        // shift row by one if dropping from above
        if (nrow < row) {
            row--;
        }
        api->movePlaylist(nrow,row);
    }
    return true;
}

