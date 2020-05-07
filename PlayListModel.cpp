#include "PlayListModel.h"

#include "QtGui.h"
#include "MainWindow.h"

PlayListModel::PlayListModel(QObject *parent, DBApi *Api) : QAbstractItemModel(parent),
                                                            DBToolbarWidget (nullptr, Api),
                                                            playIcon(":/root/images/play_16.png"),
                                                            pauseIcon(":/root/images/pause_16.png") {
    //connect(w->Api(), SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)), this, SLOT(trackChanged(DB_playItem_t*,DB_playItem_t*)));
    //connect(w->Api(), SIGNAL(playbackPaused()), this, SLOT(playerPaused()));
    columnNames.insert("%s", tr("Status"));
    columnNames.insert("%n", tr("№"));
    columnNames.insert("%t", tr("Title"));
    columnNames.insert("%a", tr("Artist"));
    columnNames.insert("%b", tr("Album"));
    columnNames.insert("%y", tr("Year"));
    columnNames.insert("%l", tr("Duration"));
    loadConfig();
}

void PlayListModel::loadConfig() {
    QString config = "%s|%n|%t|%a|%b|%y|%l";
    columns = config.split('|');
    status_column = columns.indexOf(QRegExp("^(.*(%s).*)+$"), 0);
}

void PlayListModel::saveConfig() {
    
}

int PlayListModel::columnCount(const QModelIndex &) const {
    return columns.count();
}

Qt::ItemFlags PlayListModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QVariant PlayListModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const {
    switch (role) {
    case Qt::DisplayRole: {
        ddb_playlist_t *plt = DBAPI->plt_get_curr();
        DB_playItem_t *currentDBItem = DBAPI->plt_get_item_for_idx(plt, index.row(), PL_MAIN);
        char title[1024];
        if (index.column() == status_column) {
            DBAPI->pl_format_title(currentDBItem, 0, title, sizeof(title), DB_COLUMN_PLAYING, nullptr);
            DBAPI->pl_item_unref(currentDBItem);
            DBAPI->plt_unref(plt);
            return QString::fromUtf8(title);
        }
        DBAPI->pl_format_title(currentDBItem, 0, title, sizeof(title), -1, columns[index.column()].toUtf8().data());
        DBAPI->pl_item_unref(currentDBItem);
        DBAPI->plt_unref(plt);
        return QString::fromUtf8(title);
    }
    case Qt::DecorationRole: {
        if (index.column() == status_column) {
            ddb_playlist_t *plt = DBAPI->plt_get_curr();
            DB_playItem_t *currentDBItem = DBAPI->streamer_get_playing_track();
            if (DBAPI->plt_get_item_idx(plt, currentDBItem, PL_MAIN) == index.row()) {
                DBAPI->pl_item_unref(currentDBItem);
                DBAPI->plt_unref(plt);
                if (api->isPaused())
                    return pauseIcon;
                else
                    return playIcon;
            }
            if (currentDBItem)
                DBAPI->pl_item_unref(currentDBItem);
            DBAPI->plt_unref(plt);
        }
        break;
    }
    case Qt::SizeHintRole: {
        QSize defSize;
        //TODO: get value from settings
        defSize.setHeight(25);
        return defSize;
    }
    }

    return QVariant();
}

QModelIndex PlayListModel::index(int row, int column, const QModelIndex &parent) const {
    return createIndex(row, column, nullptr);
}

QModelIndex PlayListModel::parent(const QModelIndex &child) const {
    return QModelIndex();
}

int PlayListModel::rowCount(const QModelIndex &parent) const {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int rowCount = DBAPI->plt_get_item_count(plt, PL_MAIN);
    DBAPI->plt_unref(plt);
    return rowCount;
}

QVariant PlayListModel::headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section < columns.count())
                if (columnNames[columns[section]] == tr("Status"))
                    return QVariant("");
                else
                    return QVariant(columnNames[columns[section]]);
            else
                return QVariant(columns[section]);
        }
    }
    return QVariant();
}

void PlayListModel::trackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    if (status_column != -1) {
        ddb_playlist_t *plt = DBAPI->plt_get_curr();
        emit dataChanged(createIndex(0, status_column, nullptr), createIndex(DBAPI->plt_get_item_count(plt, PL_MAIN) - 1, status_column, nullptr));
        DBAPI->plt_unref(plt);
    }
}

void PlayListModel::playerPaused() {
    DB_playItem_t *track = DBAPI->streamer_get_playing_track();
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    QModelIndex index = createIndex(DBAPI->plt_get_item_idx(plt, track, PL_MAIN), status_column, nullptr);
    emit dataChanged(index, index);
    DBAPI->pl_item_unref(track);
    DBAPI->plt_unref(plt);
}

void PlayListModel::deleteTracks(const QModelIndexList &tracks) {
    if (tracks.length() == 0)
        return;
    beginRemoveRows(QModelIndex(), tracks.first().row(), tracks.last().row());

    QModelIndex index;
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    foreach(index, tracks) {
        DBAPI->pl_set_selected(DBAPI->plt_get_item_for_idx(plt, index.row(), PL_MAIN), 1);
    }

    DBAPI->plt_delete_selected(plt);
    endRemoveRows();
    DBAPI->plt_unref(plt);
}

void PlayListModel::sort(int column, Qt::SortOrder order) {
    if (column == status_column)
        return;
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    DBAPI->plt_sort(plt, PL_MAIN, -1, columns[column].toUtf8().data(), order);
    emit dataChanged(createIndex(0, 0, nullptr), createIndex(DBAPI->plt_get_item_count(plt, PL_MAIN), columns.count(), nullptr));
    DBAPI->plt_unref(plt);
}

void PlayListModel::clearPlayList() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    beginRemoveRows(QModelIndex(), 0, DBAPI->plt_get_item_count(plt, PL_MAIN) - 1);
    DBAPI->plt_clear(plt);
    endRemoveRows();
    DBAPI->plt_unref(plt);
}

void PlayListModel::insertByURLAtPosition(const QUrl &url, int position) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int prev_track_count = DBAPI->plt_get_item_count(plt, PL_MAIN);
    api->addTracksByUrl(url, position);
    int count = DBAPI->plt_get_item_count(plt, PL_MAIN) - prev_track_count;
    DBAPI->plt_unref(plt);
    beginInsertRows(QModelIndex(), position, position + count - 1);
    endInsertRows();
}

void PlayListModel::moveItems(QList<int> indices, int before) {
    uint32_t *inds = new uint32_t[indices.length()];
    //uint32_t inds[indices.length()];
    int i;
    for (i = 0; i < indices.length(); i++) {
        inds[i] = indices[i];
    }
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int lastItem = DBAPI->plt_get_item_count(plt, PL_MAIN) - 1;
    DB_playItem_t *bef;
    if (before > lastItem) {
        DB_playItem_t *last = DBAPI->plt_get_item_for_idx(plt, lastItem, PL_MAIN);
        bef = DBAPI->pl_get_next(last, PL_MAIN);
        DBAPI->pl_item_unref(last);
    } else {
        bef = DBAPI->plt_get_item_for_idx(plt, before, PL_MAIN);
    }
    DBAPI->pl_lock();
    DBAPI->plt_move_items(plt, PL_MAIN, plt, bef, inds, indices.length());
    DBAPI->pl_unlock();
    if (bef)
        DBAPI->pl_item_unref(bef);
    DBAPI->plt_unref(plt);
    beginRemoveRows(QModelIndex(), 0, lastItem);
    endRemoveRows();
    beginInsertRows(QModelIndex(), 0, lastItem);
    endInsertRows();
}

QStringList PlayListModel::mimeTypes () const {
    QStringList qstrList;
    qstrList.append("playlist/track");
    qstrList.append("text/uri-list");
    return qstrList;
}

Qt::DropActions PlayListModel::supportedDropActions () const {
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *PlayListModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << index.row() << text;
        }
    }

    mimeData->setData("playlist/track", encodedData);
    return mimeData;
}
