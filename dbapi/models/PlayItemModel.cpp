#include "PlayItemModel.h"
#include <QSize>
#include <QMimeData>
#include <QIODevice>
#include <QDebug>
#include <QDataStream>
#include <QUrl>

#include "../Actions.h"

#include "../CoverArt.h"

#define DBAPI (manager->deadbeef)

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

const QString PlayItemModel::defaultFormat(int role) {
    QStringList def_format = {COLUMN_FORMAT_ARTISTALBUM, COLUMN_FORMAT_ARTIST, COLUMN_FORMAT_ALBUM,
                                    COLUMN_FORMAT_TITLE, COLUMN_FORMAT_YEAR, COLUMN_FORMAT_LENGTH,
                                    COLUMN_FORMAT_TRACKNUMBER, COLUMN_FORMAT_BAND, COLUMN_FORMAT_CODEC,
                                    COLUMN_FORMAT_BITRATE};
    if (role >= ItemArtistAlbum && role <= ItemBitrate) {
        return def_format[role - ItemArtistAlbum];
    }
    return QString();
}

const QString PlayItemModel::defaultTitle(int role) {
    #define _(X) (X)
    //dbtr->tr(X)
    const QStringList items = {_("Item Index"), _("♫"), _("Album Art"), _("Artist - Album"),
                     _("Artist"), _("Album"), _("Title"), _("Year"), _("Duration"), _("Track Number"),
                     _("Band / Album Artist"), _("Codec"), _("Bitrate"), _("Custom")};
    if (role == ItemPlayingDecoration ||
        role == ItemPlayingDisplay ||
        role == ItemPlayingState) {
        role = ItemPlaying;
    }
    if (role >= ItemIndex && role <= PlayItemRoleLast + 1) {
        return items[role - ItemIndex];
    }
    return tr("Custom");
    //return QString();
}

PlayItemModel::PlayItemModel(QObject *parent, PlaylistManager *Manager) : QAbstractItemModel(parent) {
    manager = Manager;

    // Events to update current status next to playing track
    connect(manager, &PlaylistManager::statusRowChanged,
            this,    &PlayItemModel::onPlaybackChanged);
    //connect(api, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

    if (manager->dynamicPropertyNames().contains("_dbapi_cover")) {
        CoverArt *cover = manager->property("_dbapi_cover").value<CoverArt *>();
        if (cover) {
            source_id = cover->allocateSourceId();
            connect(cover, &ICoverArtCache::coverArtChanged, this, &PlayItemModel::onCoverArtChanged);
        }
    }

    // compile default formats
    for (int i = ItemArtistAlbum; i <= ItemBitrate; i++) {
        char *fmt_compiled = DBAPI->tf_compile(defaultFormat(i).toUtf8());
        if (!fmt_compiled) {
            qDebug() << "PlayItemModel: compiling default format failed!";
        }
        format_map.insert(i, fmt_compiled);
    }
}

PlayItemModel::~PlayItemModel() {
    // clean format_map
    for (QHash<int, char*>::const_iterator it = format_map.cbegin(), end = format_map.cend(); it != end; ++it) {
        DBAPI->tf_free(it.value());
    }
    format_map.clear();
}

void PlayItemModel::insertTracks(playItemList *l, int after) {
    Q_UNUSED(l)
    Q_UNUSED(after)
    qDebug() << "PlayItemModel::insertTracks unimplemented!";
}

void PlayItemModel::moveIndexes(QList<int> indices, int after) {
    Q_UNUSED(indices)
    Q_UNUSED(after)
    qDebug() << "PlayItemModel::moveIndexes unimplemented!";
}

void PlayItemModel::removeIndexes(QList<int> indices) {
    Q_UNUSED(indices)
    qDebug() << "PlayItemModel::removeIndexes unimplemented!";
}

int PlayItemModel::addFormat(QString str) {
    if (format_role_map.contains(str)) {
        return format_role_map.value(str);
    }
    char *fmt_compiled = DBAPI->tf_compile(str.toUtf8());
    if (!fmt_compiled) {
        qDebug() << "PlayItemModel: compiling format failed!";
        return -1;
    }
    int role = custom_role_last;
    custom_role_last++;

    format_role_map.insert(str, role);
    format_map.insert(role, fmt_compiled);
    return role;
}

int PlayItemModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    if (parent.isValid()) {
        return 0;
    }
    return 1;
}

Qt::ItemFlags PlayItemModel::flags(const QModelIndex &index) const {
    // NOTE: item never has children flag is enabled here
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index) | Qt::ItemNeverHasChildren;

    if (index.isValid()) {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}

QModelIndex PlayItemModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return createIndex(row, column, nullptr);
}

QModelIndex PlayItemModel::parent(const QModelIndex &child) const {
    Q_UNUSED(child);
    return QModelIndex();
}

QString PlayItemModel::itemTfParse(DB_playItem_t *it, int role) const {
    // todo return qvariant
    if (!format_map.contains(role)) {
        qDebug() << "PlayItemModel: parsing of role" << role << "requested, but no compiled format for that role available!";
        return QString();
    }
    // compile
    char buffer[1024];
    ddb_tf_context_t context;
    {
        memset(&context, 0, sizeof(context));
        context._size = sizeof(ddb_tf_context_t);
        context.it = it;
        //if (context.it) {
        //    context.plt = DBAPI->pl_get_playlist(context.it);
        //}
        //else {
            context.plt = nullptr;
        //}
        // TODO: m_iter better handling?
        context.iter = m_iter;
        context.iter = m_iter;
    }
    DBAPI->tf_eval (&context, format_map.value(role), buffer, 1024);
    return QString(buffer);
}

QVariant PlayItemModel::data(const QModelIndex &index, int role) const {
    QVariant ret;
    if (index.isValid()) {
        DB_playItem_t *it = track(index);
        if (!it) {
            return QVariant();
        }
        switch (role) {
            case ItemPlayingState:
                {
                    DB_playItem_t *curr = DBAPI->streamer_get_playing_track();
                    if (curr) {
                        if (curr == it) {
                            // TODO might make issues
                            DB_output_t *out = DBAPI->get_output();
                            if (out && out->state() == DDB_PLAYBACK_STATE_PAUSED)
                                ret = 2;
                            else
                                ret = 1;
                        }
                        else {
                            ret = 0;
                        }
                        DBAPI->pl_item_unref(curr);
                    }
                    else {
                        ret = 0;
                    }
                }
                break;
            case ItemSelected:
                ret = DBAPI->pl_is_selected(it);
                break;
            case ItemNumber:
                ret = QString::number(index.row() + 1);
                break;
            case ItemIndex:
                // index guaranteed to be row of this model
                ret = QString::number(index.row());
                break;
            case ItemQueue:
                if ((DBAPI->playqueue_test(it) != -1)) {
                    QList<int> in_queue;
                    int i;
                    for (i = 0; i < DBAPI->playqueue_get_count(); i++) {
                        DB_playItem_t *it_test = DBAPI->playqueue_get_item(i);
                        if (it_test == it) {
                            in_queue.append(i+1);
                        }
                        DBAPI->pl_item_unref(it_test);
                    }
                    ret = QVariant::fromValue(in_queue);
                }
                break;
            case ItemPlaying:
            case ItemPlayingDisplay:
                // TODO fix queue hack
                /*if (property("queueManager").toBool()) {
                  toMimeData  ret = QString("%1") .arg(index.row()+1);
                }*/
                if ((DBAPI->playqueue_test(it) != -1)) {
                    QList<int> in_queue;
                    int i;
                    for (i = 0; i < DBAPI->playqueue_get_count(); i++) {
                        DB_playItem_t *it_test = DBAPI->playqueue_get_item(i);
                        if (it_test == it) {
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
            case ItemAlbumArtUrl:
            case ItemAlbumArt:
                if (manager->dynamicPropertyNames().contains("_dbapi_cover")) {
                    ICoverArtCache *cover = manager->property("_dbapi_cover").value<ICoverArtCache *>();
                    if (cover) {
                        CoverArtType type = (role == ItemAlbumArt) ?
                                                COVER_QIMAGE : COVER_QSTRING;
                        ICoverArtCache::CoverArtRequest_t req = {it,QSize(),type, source_id, index.row()};
                        CoverArtStatusFlags flags = cover->coverArtStatus(req);
                        if (flags & STATUS_MISS || flags & STATUS_CACHED) {
                            ret = cover->getCoverArt(req);
                        }
                    }
                }
                break;
            case ItemMime:
                ret = QVariant::fromValue(mimeData({index}));
                break;
            case ItemIterator:
                ret = QVariant::fromValue(PlayItemIterator(it));
                break;
            case ItemCursor: {
                DB_playItem_t *it_test = DBAPI->pl_get_for_idx(DBAPI->pl_get_cursor(PL_MAIN));
                if (it_test) {
                    ret = it_test == it;
                    DBAPI->pl_item_unref(it_test);
                }
                break;
            }
            case Qt::SizeHintRole:
            {
                QSize defSize;
                //TODO: get value from settings
                defSize.setHeight(25);
                ret = defSize;
                break;
            }
            default:
                if (role >= ItemArtistAlbum) {
                    ret = itemTfParse(it, role);
                }
                break;
        }
        DBAPI->pl_item_unref(it);
    }
    return ret;
}

QVariant PlayItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role >= ItemIndex && role <= PlayItemRoleLast + 1 || role >= CustomRoleStart) {
            return defaultTitle(role);
        }
        else if (role == Qt::SizeHintRole) {
            return QVariant(QSize(-1,25));
        }
    }
    return QVariant();
}


void PlayItemModel::onPlaybackChanged() {
    emit dataChanged(createIndex(0,0), createIndex(rowCount(),columnCount()), QVector<int>{ItemPlayingState, ItemPlaying,
                                                                                           ItemPlayingDisplay, ItemPlayingDecoration,
                                                                                           ItemQueue});
}

void PlayItemModel::onSelectionChanged() {
    // TODO: better solution?
    emit dataChanged(createIndex(0,0), createIndex(rowCount(), columnCount(QModelIndex())),QVector<int>{ItemSelected});
}

void PlayItemModel::onCoverArtChanged(int type, int id, qint64 userdata) {
    if (id == source_id) {
        /*
        int role;
        switch (type) {
            case COVER_QIMAGE:
                role = ItemAlbumArt;
                break;
            case COVER_QSTRING:
                role = ItemAlbumArtUrl;
                break;
            default:
                role = 0;
        }
        if (role) {*/
            emit dataChanged(createIndex(userdata,0), createIndex(userdata, columnCount(QModelIndex())),QVector<int>{ItemAlbumArt, ItemAlbumArtUrl});
        //}

    }
}

QStringList PlayItemModel::mimeTypes () const {
    QStringList qstrList = {"ddb_gui_q/playitemdata"};
    return qstrList;
}

Qt::DropActions PlayItemModel::supportedDropActions () const {
    if (!m_playlistLock)
        return Qt::CopyAction | Qt::MoveAction;
    return Qt::IgnoreAction;
}

QMimeData *PlayItemModel::mimeData(const QModelIndexList &indexes) const {
    playItemList l = tracks(indexes);
    QMimeData *data = new PlayItemMimeData(DBAPI, l);
    foreach (DB_playItem_t *it, l) {
        DBAPI->pl_item_unref(it);
    }
    return data;
}

bool PlayItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (data->hasFormat("ddb_gui_q/playitemdata") && !parent.isValid()) {
        QList<DB_playItem_t*> tracks = PlayItemMimeData::getTracks(data);
        if (row == -1) {
            insertTracks(&tracks, -2);
        }
        else {
            insertTracks(&tracks, row);
        }
        return true;
    }
    return false;
}

bool PlayItemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    if (data->hasFormat("ddb_gui_q/playitemdata")) {
        return true;
    }
    return false;
}

int PlayItemModel::getIter() const {
    return m_iter;
}

void PlayItemModel::setIter(int iter) {
    beginResetModel();
    m_iter = iter;
    emit iterChanged();
    endResetModel();
}

bool PlayItemModel::getPlaylistLock() const {
    return m_playlistLock;
}
void PlayItemModel::setPlaylistLock(bool lock) {
    if (m_playlistLock != lock) {
        m_playlistLock = lock;
        emit playlistLockChanged();
    }
}

QHash<int, QByteArray> PlayItemModel::roleNames() const {
    QHash<int, QByteArray> l;
    l.insert(Qt::DisplayRole, "ItemDisplay");
    l.insert(ItemPlayingState, "ItemPlayingState");
    l.insert(ItemSelected, "ItemSelected");
    l.insert(ItemPlaying,"ItemPlaying");
    l.insert(ItemPlayingState,"ItemPlayingState");
    l.insert(ItemIndex,"ItemIndex");
    l.insert(ItemPlaying,"ItemPlaying");
    l.insert(ItemAlbumArt,"ItemAlbumArt");
    l.insert(ItemAlbumArtUrl,"ItemAlbumArtUrl");
    l.insert(ItemArtistAlbum,"ItemArtistAlbum");
    l.insert(ItemArtist,"ItemArtist");
    l.insert(ItemAlbum,"ItemAlbum");
    l.insert(ItemTitle,"ItemTitle");
    l.insert(ItemYear,"ItemYear");
    l.insert(ItemLength,"ItemLength");
    l.insert(ItemTrackNum,"ItemTrackNum");
    l.insert(ItemBandAlbumArtist,"ItemBandAlbumArtist");
    l.insert(ItemCodec,"ItemCodec");
    l.insert(ItemBitrate,"ItemBitrate");
    l.insert(ItemMime, "ItemMime");
    l.insert(ItemIterator, "ItemIterator");
    l.insert(ItemCursor, "ItemCursor");
    l.insert(ItemQueue, "ItemQueue");
    //l.insert(LastRoleUnused,"LastRoleUnused");
    return l;
}

CurrentPlayItemModel::CurrentPlayItemModel(QObject *parent, PlaylistManager *Manager) : PlayItemModel(parent,Manager) {
    connect(manager, SIGNAL(statusRowChanged()), this, SLOT(onPlaybackChanged()));
}

int CurrentPlayItemModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 1;
}

playItemList CurrentPlayItemModel::tracks(const QList<int> &tracks) const {
    Q_UNUSED(tracks)
    return playItemList{DBAPI->streamer_get_playing_track()};
}

void CurrentPlayItemModel::onPlaybackChanged() {
    emit dataChanged(createIndex(0,0),createIndex(0,0));
}
