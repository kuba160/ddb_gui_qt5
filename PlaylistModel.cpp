#include "PlaylistModel.h"

#include "QtGui.h"
#include "MainWindow.h"
#include "DeadbeefTranslator.h"

PlaylistModel::PlaylistModel(QObject *parent, DBApi *Api) : QAbstractItemModel(parent),
                                                            DBWidget(nullptr,Api),
                                                            playIcon(":/root/images/play_16.png"),
                                                            pauseIcon(":/root/images/pause_16.png") {
    // Events to show current status next to playing track
    connect(api, SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)), this, SLOT(onTrackChanged(DB_playItem_t*,DB_playItem_t*)));
    connect(api, SIGNAL(playbackPaused()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(playbackUnPaused()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(queueChanged()), this, SLOT(onPlaybackChanged()));
}

PlaylistModel::~PlaylistModel() {
    if (plt) {
        DBAPI->plt_unref(plt);
    }
}

void PlaylistModel::setPlaylist(ddb_playlist_t *plt_new) {
    emit beginResetModel();
    if (plt) {
        DBAPI->plt_unref(plt);
    }
    plt = plt_new;
    if (plt) {
        DBAPI->plt_ref(plt);
    }
    emit endResetModel();
}

ddb_playlist_t *PlaylistModel::getPlaylist() {
    DBAPI->plt_ref(plt);
    return plt;
}

void PlaylistModel::setPlaylistLock(bool lock) {
    isLocked = lock;
}

void PlaylistModel::modelBeginReset() {
    emit beginResetModel();
}

void PlaylistModel::modelEndReset() {
    emit endResetModel();
}


void PlaylistModel::setColumns(QList<PlaylistHeader_t *> &c_new) {
    emit beginResetModel();
    columns = c_new;
    int i;
    for (i = 0; i < columns.size(); i++) {
        if (columns.at(i)->type != HT_custom) {
            columns.at(i)->format = QString(formatFromHeaderType(columns.at(i)->type));
            if (!columns.at(i)->format.isEmpty()) {
                columns.at(i)->_format_compiled = DBAPI->tf_compile(columns.at(i)->format.toUtf8());
            }
        }
    }
    emit columnsChanged();
    emit endResetModel();
    /*if (columns != c_new) {
        int i;
        for (i = 0; i < columns.size(); i++) {
            if (columns.at(i)->_format_compiled) {
                DBAPI->tf_free(columns.at(i)->_format_compiled);
            }
            delete columns.at(i);
        }
        columns = c_new;
        emit columnsChanged();
    }*/
}

// Taken from DeaDBeeF gtkui (plcommon.h)
//#define COLUMN_FORMAT_ARTISTALBUM "$if(%artist%,%artist%,Unknown Artist)[ - %album%]"
#define COLUMN_FORMAT_ARTISTALBUM "$if(%album artist%,%album artist%,$if(%artist%,%artist%,Unknown Artist))[ - %album%]"
#define COLUMN_FORMAT_ARTIST "$if(%artist%,%artist%,Unknown Artist)"
#define COLUMN_FORMAT_ALBUM "%album%"
#define COLUMN_FORMAT_TITLE "%title%"
#define COLUMN_FORMAT_YEAR "%year%"
#define COLUMN_FORMAT_LENGTH "%length%"
#define COLUMN_FORMAT_TRACKNUMBER "%tracknumber%"
#define COLUMN_FORMAT_BAND "$if(%album artist%,%album artist%,Unknown Artist)"
#define COLUMN_FORMAT_CODEC "%codec%"
#define COLUMN_FORMAT_BITRATE "%bitrate%"

QString PlaylistModel::formatFromHeaderType(headerType t) {
    // no format for HT_empty, HT_itemIndex, HT_playing and HT_albumArt
    const QString map[] = {"", "", "", "", COLUMN_FORMAT_ARTISTALBUM, COLUMN_FORMAT_ARTIST, COLUMN_FORMAT_ALBUM, COLUMN_FORMAT_TITLE,
                           COLUMN_FORMAT_YEAR, COLUMN_FORMAT_LENGTH, COLUMN_FORMAT_TRACKNUMBER, COLUMN_FORMAT_BAND, COLUMN_FORMAT_CODEC, COLUMN_FORMAT_BITRATE };
    if (t < HT_custom && t > HT_empty) {
        return map[t];
    }
    return "";
}

QStringList items = {_("Item Index"), _("Playing"), _("Album Art"), _("Artist - Album"),
                     _("Artist"), _("Album"), _("Title"), _("Year"), _("Duration"), _("Track Number"),
                     _("Band / Album Artist"), _("Codec"), _("Bitrate"), _("Custom")};

QString PlaylistModel::titleFromHeaderType(headerType t) {
    const QStringList items = {_("Item Index"), _("Playing"), _("Album Art"), _("Artist - Album"),
                         _("Artist"), _("Album"), _("Title"), _("Year"), _("Duration"), _("Track Number"),
                         _("Band / Album Artist"), _("Codec"), _("Bitrate"), _("Custom")};
    if (t < HT_custom+1 && t > HT_empty+1) {
        return items.at(t-1);
    }
    return "";
}

QList<PlaylistHeader_t *> *PlaylistModel::setDefaultHeaders() {
    QList<PlaylistHeader_t *> default_headers;
        //  Title, type, formatting
    PlaylistHeader_t a[] = {
        {"♫", HT_playing,""},
        {_("Artist - Album"), HT_artistAlbum,""},
        {_("Track No"), HT_trackNum,""},
        {_("Title"), HT_title,""},
        {_("Duration"), HT_length, ""},
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
    return &columns;
}

int PlaylistModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return columns.count();
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (isLocked) {
        return defaultFlags;
    }
    else if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
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
            case HT_itemIndex:
                // TODO start at 1?
                ret = QString::number(index.row());
                break;
            case HT_playing:
                // TODO include information about queue here
                if ((DBAPI->playqueue_test(context.it) != -1)) {
                    QList<int> in_queue;
                    int i;
                    for (i = 0; i < DBAPI->playqueue_get_count(); i++) {
                        DB_playItem_t *it_test = DBAPI->playqueue_get_item(i);
                        if (it_test == context.it) {
                            in_queue.append(i);
                        }
                        DBAPI->pl_item_unref(it_test);
                    }
                    QString s = "(";
                    for (i = 0; i < in_queue.count(); i++) {
                        s += QString("%1") .arg(in_queue[i] + 1);
                        if (i+1 != in_queue.count())
                            s += ",";
                    }
                    s += ")";
                    ret = s;
                }
                break;
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
                    DBAPI->pl_lock();
                    DBAPI->tf_eval (&context, h->_format_compiled, buffer, 1024);
                    DBAPI->pl_unlock();
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
    int rowCount = DBAPI->plt_get_item_count(plt, PL_MAIN);
    DBAPI->pl_unlock ();
    return rowCount;
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole && section < columns.count()) {
            return QVariant(columns[section]->title);
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
    if (tracks.length() == 0 || isLocked)
        return;
    beginRemoveRows(QModelIndex(), tracks.first().row(), tracks.last().row());

    QModelIndex index;
    foreach(index, tracks) {
        DBAPI->pl_set_selected(DBAPI->plt_get_item_for_idx(plt, index.row(), PL_MAIN), 1);
    }

    DBAPI->plt_delete_selected(plt);
    endRemoveRows();
}

void PlaylistModel::sort(int n, Qt::SortOrder order) {
    if (!plt || n >= columns.length() || columns[n]->type == HT_playing || columns[n]->format.isEmpty() || isLocked) {
        return;
    }
    DBAPI->plt_sort_v2(plt, PL_MAIN, -1, columns[n]->format.toUtf8(), order);
    emit dataChanged(createIndex(0, 0, nullptr), createIndex(DBAPI->plt_get_item_count(plt, PL_MAIN), columns.count(), nullptr));
}

void PlaylistModel::clearPlayList() {
    beginRemoveRows(QModelIndex(), 0, DBAPI->plt_get_item_count(plt, PL_MAIN) - 1);
    DBAPI->plt_clear(plt);
    endRemoveRows();
}

void PlaylistModel::insertByURLAtPosition(const QUrl &url, int position) {
    int prev_track_count = DBAPI->plt_get_item_count(plt, PL_MAIN);
    api->addTracksByUrl(url, position);
    int count = DBAPI->plt_get_item_count(plt, PL_MAIN) - prev_track_count;
    beginInsertRows(QModelIndex(), position, position + count - 1);
    endInsertRows();
}

void PlaylistModel::insertByPlayItemAtPosition(DB_playItem_t *item, int position) {
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
    beginInsertRows(QModelIndex(), position, position + count - 1);
    endInsertRows();
}
void PlaylistModel::moveItems(QList<int> indices, int before) {
    // TODO FIX THIS
    uint32_t *inds = new uint32_t[indices.length()];
    //uint32_t inds[indices.length()];
    int i;
    for (i = 0; i < indices.length(); i++) {
        inds[i] = indices[i];
    }
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
    beginRemoveRows(QModelIndex(), 0, lastItem);
    endRemoveRows();
    beginInsertRows(QModelIndex(), 0, lastItem);
    endInsertRows();
    emit rowsChanged();
}

QStringList PlaylistModel::mimeTypes () const {
    QStringList qstrList = {"deadbeef/playitems"};
    return qstrList;
}

Qt::DropActions PlaylistModel::supportedDropActions () const {
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData *PlaylistModel::mimeData(const QModelIndexList &indexes) const {
    QList<DB_playItem_t *> items;
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                DB_playItem_t *it = DBAPI->plt_get_item_for_idx(plt,index.row(),PL_MAIN);
                items.append(it);
                rows.append(index.row());
            }
        }
    }

    return api->mime_playItems(items);
}

QDataStream &operator<<(QDataStream &ds, const PlaylistHeader_t &pil) {
    ds << pil.title;
    ds << pil.type;
    if (pil.type == HT_custom)
        ds << pil.format;
    else
        ds << QString();
    return ds;
}
QDataStream& operator >> (QDataStream &ds, PlaylistHeader_t &pil) {
    ds >> pil.title;
    ds >> pil.type;
    ds >> pil.format;
    pil._format_compiled = nullptr;
    return ds;
}
