#include "PlaylistModel.h"

#include "QtGui.h"
#include "MainWindow.h"

PlaylistModel::PlaylistModel(QObject *parent, DBApi *Api) : QAbstractItemModel(parent),
                                                            DBWidget(nullptr,Api),
                                                            playIcon(":/root/images/play_16.png"),
                                                            pauseIcon(":/root/images/pause_16.png") {
    // Events to show current status next to playing track
    connect(api, SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)), this, SLOT(onTrackChanged(DB_playItem_t*,DB_playItem_t*)));
    connect(api, SIGNAL(playbackPaused()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(playbackUnPaused()), this, SLOT(onPlaybackChanged()));
}

PlaylistModel::~PlaylistModel() {
    if (plt) {
        DBAPI->plt_unref(plt);
    }
}

void PlaylistModel::setPlaylist(ddb_playlist_t *plt_new) {
    if (plt) {
        DBAPI->plt_unref(plt);
    }
    plt = plt_new;
    DBAPI->plt_ref(plt);
}

void PlaylistModel::setColumns(QList<PlaylistHeader_t *> &c_new) {
    if (columns != c_new) {
        int i;
        for (i = 0; i < columns.size(); i++) {
            if (columns.at(i)->_format_compiled) {
                DBAPI->tf_free(columns.at(i)->_format_compiled);
            }
            delete columns.at(i);
        }
        columns = c_new;
        emit columnsChanged();
    }
}

QString PlaylistModel::formatFromHeaderType(headerType t) {
    const QString map[] = {"","%list_index%","", "", "%artist% / %album%", "%artist%", "%album%", "%title%",
                           "%year%", "%tracknumber%", "%album artist% / %album%", "%codec%", "%bitrate%", "INVALID"};
    if (t < HT_custom) {
        return map[t];
    }
    return "";
}

QList<PlaylistHeader_t *> PlaylistModel::setDefaultHeaders() {
    QList<PlaylistHeader_t *> default_headers;
        //  Title, type, formatting
    PlaylistHeader_t a[] = {
        {"♫", HT_playing,"",1},
        {"Artist / Album", HT_artistAlbum,"",1},
        {"Track No", HT_trackNum,"",1},
        {"Title", HT_title,"",1},
        {"Duration", HT_custom, "%length%",1},
        {"", HT_empty}
    };
    int i;
    for (i = 0; a[i].type != HT_empty; i++) {
        PlaylistHeader_t *temp = new PlaylistHeader_t;
        *temp = a[i];
        if (temp->type == HT_custom) {
            temp->_format_compiled = DBAPI->tf_compile(a[i].format.toUtf8());
        }
        else {
            temp->format = formatFromHeaderType(temp->type);
            if (!temp->format.isEmpty()) {
                temp->_format_compiled = DBAPI->tf_compile(temp->format.toUtf8());
            }
        }
        default_headers.append(temp);
    }
    setColumns(default_headers);
    return columns;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return columns.count();
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    }
    else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}

QVariant PlaylistModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const {
    QVariant ret;
    PlaylistHeader_t *h = columns[index.column()];
    switch (role) {
    case Qt::DisplayRole:
        if (plt) {
            ddb_tf_context_t context;
            {
                context._size = sizeof(ddb_tf_context_t);
                context.flags = 0;
                context.it = DBAPI->plt_get_item_for_idx(plt, index.row(), PL_MAIN);
                context.plt = plt;
                context.idx = 0;
                context.id = 0;
                context.iter = PL_MAIN;
                context.update = 0;
                context.dimmed = 0;
            }
            char buffer[1024]; // TODO hardcoded 1024
            buffer[0] = 0;
            switch (h->type) {
            case HT_playing:
                // TODO include information about queue here
                //ret = QString::fromUtf8("");
                //break;
            case HT_albumArt:
                ret = QString::fromUtf8("");
                break;
            case HT_custom:
                if (!h->_format_compiled) {
                    columns.at(index.column())->_format_compiled = DBAPI->tf_compile(h->format.toUtf8());
                }
                /* fall through */
            default:
                if (h->_format_compiled) {
                    DBAPI->tf_eval (&context, h->_format_compiled, buffer, 1024);
                    ret = QString::fromUtf8(buffer);
                }
                break;
            }
            if (context.it) {
                DBAPI->pl_item_unref(context.it);
            }
        }
        break;
    case Qt::DecorationRole:
        if (h->type == HT_playing) {
            DB_playItem_t *curr = DBAPI->streamer_get_playing_track();
            if (curr) {
                int idx = DBAPI->plt_get_item_idx(plt, curr, PL_MAIN);
                if (idx == index.row()) {
                    if (api->isPaused())
                        ret = pauseIcon;
                    else
                        ret = playIcon;
                }
                DBAPI->pl_item_unref(curr);
            }
        }
        break;
    case Qt::SizeHintRole:
        QSize defSize;
        //TODO: get value from settings
        defSize.setHeight(25);
        ret = defSize;
        break;
    }
    return ret;
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return createIndex(row, column, nullptr);
}

QModelIndex PlaylistModel::parent(const QModelIndex &child) const {
    Q_UNUSED(child);
    return QModelIndex();
}

int PlaylistModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    DBAPI->pl_lock ();
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int rowCount = DBAPI->plt_get_item_count(plt, PL_MAIN);
    DBAPI->plt_unref(plt);
    DBAPI->pl_unlock ();
    return rowCount;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole && section < columns.count()) {
            if (columns[section]->_translate) {
                return QVariant(QString(_(columns[section]->title.toUtf8())));
            }
            else {
                return QVariant(columns[section]->title);
            }
        }
    }
    return QVariant();
}

void PlaylistModel::onPlaybackChanged() {
    // Do not update if different playlist
    ddb_playlist_t *plt_curr = DBAPI->plt_get_curr();
    if (plt != plt_curr) {
        DBAPI->plt_unref(plt_curr);
        return;
    }
    DBAPI->plt_unref(plt_curr);
    int i;
    for (i = 0; i < columns.count(); i++) {
        if (columns.at(i)->type == HT_playing) {
            // update whole column
            emit dataChanged(createIndex(0, i, nullptr), createIndex(DBAPI->plt_get_item_count(plt, PL_MAIN) - 1, i, nullptr));
        }
    }
}

void PlaylistModel::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    Q_UNUSED(from); Q_UNUSED(to)
    // playback change
    onPlaybackChanged();
}

void PlaylistModel::deleteTracks(const QModelIndexList &tracks) {
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

void PlaylistModel::sort(int n, Qt::SortOrder order) {
    if (n >= columns.length() || columns[n]->type == HT_playing) {
        return;
    }
    DBAPI->plt_sort_v2(plt, PL_MAIN, -1, columns[n]->format.toUtf8(), order);
    emit dataChanged(createIndex(0, 0, nullptr), createIndex(DBAPI->plt_get_item_count(plt, PL_MAIN), columns.count(), nullptr));
}

void PlaylistModel::clearPlayList() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    beginRemoveRows(QModelIndex(), 0, DBAPI->plt_get_item_count(plt, PL_MAIN) - 1);
    DBAPI->plt_clear(plt);
    endRemoveRows();
    DBAPI->plt_unref(plt);
}

void PlaylistModel::insertByURLAtPosition(const QUrl &url, int position) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int prev_track_count = DBAPI->plt_get_item_count(plt, PL_MAIN);
    api->addTracksByUrl(url, position);
    int count = DBAPI->plt_get_item_count(plt, PL_MAIN) - prev_track_count;
    DBAPI->plt_unref(plt);
    beginInsertRows(QModelIndex(), position, position + count - 1);
    endInsertRows();
}

void PlaylistModel::insertByPlayItemAtPosition(DB_playItem_t *item, int position) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int prev_track_count = DBAPI->plt_get_item_count(plt, PL_MAIN);
    DB_playItem_t *after;
    if (position == -1) {
        after = nullptr;
    }
    else {
        after = DBAPI->pl_get_for_idx(position);
    }
    DBAPI->plt_insert_item(plt,after,item);
    int count = DBAPI->plt_get_item_count(plt, PL_MAIN) - prev_track_count;
    DBAPI->plt_unref(plt);
    beginInsertRows(QModelIndex(), position, position + count - 1);
    endInsertRows();
}
void PlaylistModel::moveItems(QList<int> indices, int before) {
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

QStringList PlaylistModel::mimeTypes () const {
    QStringList qstrList;
    qstrList.append("playlist/track");
    qstrList.append("medialib/tracks");
    qstrList.append("text/uri-list");
    return qstrList;
}

Qt::DropActions PlaylistModel::supportedDropActions () const {
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const
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
