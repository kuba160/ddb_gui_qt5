#include <QIODevice>
#include <QRegularExpression>
#include "PlaylistBrowserModel.h"

#include <QDebug>

#define DBAPI (manager->deadbeef)

PlaylistBrowserModel::PlaylistBrowserModel(QObject *parent, PlaylistManager *Manager) : QAbstractListModel(parent) {
    manager = Manager;

    //connect(api, SIGNAL(playlistMoved(int,int)), this, SLOT(onPlaylistMoved(int,int)));
    //connect(api, SIGNAL(playlistCreated()), this, SLOT(onPlaylistCreated()));
    //connect(api, SIGNAL(playlistRenamed(int)), this, SLOT(onPlaylistRenamed(int)));
    //connect(api, SIGNAL(playlistRemoved(int)), this, SLOT(onPlaylistRemoved(int)));
}

PlaylistBrowserModel::~PlaylistBrowserModel() {

}

int PlaylistBrowserModel::rowCount(const QModelIndex &parent) const {
    return DBAPI->plt_get_count();
    if (!parent.isValid()) {
        return DBAPI->plt_get_count();
    }
    qDebug() << "rowCount invalid";
    return 0;
}

QVariant PlaylistBrowserModel::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {
        if (role == Qt::DisplayRole || role == PlaylistNameRole) {
            if (index.row() <= DBAPI->plt_get_count()) {
                ddb_playlist_t *plt = DBAPI->plt_get_for_idx(index.row());
                if (plt) {
                    char buf[512];
                    DBAPI->plt_get_title(plt, buf, 512);
                    DBAPI->plt_unref(plt);
                    return QVariant(QString(buf));
                }
            }
        }
        else {
            // TODO implement other roles (playlistItems, playlistLength)
        }
    }
    return QVariant();
}

bool PlaylistBrowserModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole && value.canConvert<QString>()) {
        if (index.row() < DBAPI->plt_get_count()) {
            ddb_playlist_t *plt = DBAPI->plt_get_for_idx(index.row());
            if (plt) {
                DBAPI->plt_set_title(plt, value.toString().toUtf8().constData());
                emit dataChanged(index,index);
                DBAPI->plt_unref(plt);
            }
        }
    }
    return false;
}

// new playlist insert
bool PlaylistBrowserModel::insertRows(int row, int count, const QModelIndex &parent) {
    if (parent.isValid() || row > DBAPI->plt_get_count() || row < 0 || count < 0) {
        return false;
    }
    // count current playlists
    int plt_new_count = 0;
    QString name = tr("New Playlist");
    for (int i = 0; i < DBAPI->plt_get_count(); i++) {
        char buf[512];
        DBAPI->plt_get_title(DBAPI->plt_get_for_idx(i), buf, 512);
        QRegularExpression re(name + "( \\([1-9]+\\))?");
        if (re.match(buf).hasMatch()) {
            count++;
        }
    }

    beginInsertRows(QModelIndex(), row, row+count);
    while (count) {
        QString name_real = name;
        if (plt_new_count) {
            name_real.append(" (" + QString("a") + ")");
        }
        int pos = (row == 0) ? DBAPI->plt_get_count() : row;
        DBAPI->plt_add(pos, name_real.toUtf8().constData());
        plt_new_count++;
        row++;
        count--;
    }
    endInsertRows();
    return true;
}

// move playlist
bool PlaylistBrowserModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) {
    if (sourceParent.isValid() || destinationParent.isValid() ||
        count < 0 || sourceRow >= DBAPI->plt_get_count() || sourceRow < 0) {
        return false;
    }

    if (sourceRow < 0
        || sourceRow + count - 1 >= rowCount(sourceParent)
        || destinationChild > rowCount(destinationParent)
        || sourceRow == destinationChild - 1
        || count <= 0) {
        qDebug() << "WRONG MOVEROWS!!";
        return false;
    }

    beginMoveRows({}, sourceRow, sourceRow - 1 + count, {}, destinationChild);
    while (count) {
        // todo index 0 = move back
        DBAPI->plt_move(sourceRow,destinationChild);
        sourceRow++;
        count--;
    }
    endMoveRows();
    return true;
}

bool PlaylistBrowserModel::removeRows(int row, int count, const QModelIndex &parent) {
    if (parent.isValid() || row < 0 || row >= DBAPI->plt_get_count()) {
        return false;
    }

    beginRemoveRows(parent, row, row-1+count);

    while (count) {
        DBAPI->plt_remove(row);
        count--;
        row++;
    }
    endRemoveRows();

    return true;

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
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
    /*
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
    return true;*/
}

QHash<int, QByteArray> PlaylistBrowserModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[PlaylistNameRole] = "playlistName";
    roles[PlaylistItemsRole] = "playlistItems";
    roles[PlaylistLengthRole] = "playlistLength";
    return roles;
}
