#include "PlayItemModel.h"

#include "MainWindow.h"
#include "DeadbeefTranslator.h"

#include "QtGui.h"

PlayItemModel::PlayItemModel(QObject *parent, DBApi *api_a) : QAbstractItemModel(parent),
                                                DBWidget(nullptr,api_a),
                                                playIcon(":/root/images/play_16.png"),
                                                pauseIcon(":/root/images/pause_16.png") {


    if (api_a == nullptr) {
        this->api = getDBApi();
    }
    // Events to show current status next to playing track
    connect(api, SIGNAL(trackChanged(DB_playItem_t*,DB_playItem_t*)),
            this, SLOT(onTrackChanged(DB_playItem_t*,DB_playItem_t*)));
    connect(api, SIGNAL(playbackPaused()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(playbackUnPaused()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(queueChanged()), this, SLOT(onPlaybackChanged()));
}



PlayItemModel::~PlayItemModel() {

}

void PlayItemModel::compileFormat(PlaylistHeader_t *h) {
    if (h->type != HT_custom) {
        h->format = QString(formatFromHeaderType((headerType) h->type));
    }
    if (!h->format.isEmpty()) {
        h->_format_compiled = DBAPI->tf_compile(h->format.toUtf8());
    }
}

void PlayItemModel::setColumns(QList<PlaylistHeader_t *> *c_new) {
    //emit beginResetModel();
    // free compiled tf
    if (columns.length()) {
        foreach(PlaylistHeader_t *h, columns) {
            if (h->_format_compiled) {
                DBAPI->tf_free(h->_format_compiled);
            }
            delete h;
        }
    }

    columns = *c_new;
    // use default if empty
    if (c_new->empty()) {
        columns = *defaultHeaders();
    }
    // rebuild tf
    beginResetModel();
    for (int i = 0; i < columns.size(); i++) {
        PlaylistHeader_t *h = columns.at(i);
        compileFormat(h);
    }
    endResetModel();
    emit columnsChanged();
    //emit
}

void PlayItemModel::addColumn(PlaylistHeader_t *c, int before) {
    if (before == -1) {
        beginInsertColumns(QModelIndex(),columnCount(QModelIndex()),columnCount(QModelIndex()));
        columns.append(c);
    }
    else {
        beginInsertColumns(QModelIndex(),before,before);
        columns.insert(before,c);
    }
    compileFormat(c);
    emit columnsChanged();
    endInsertColumns();
}

void PlayItemModel::removeColumn(int i) {
    if (i >= columns.length()) {
        return;
    }
    beginRemoveColumns(QModelIndex(),i,i);
    PlaylistHeader_t *c = columns.takeAt(i);
    DBAPI->tf_free(c->_format_compiled);
    delete c;
    endRemoveColumns();
}

void PlayItemModel::replaceColumn(int i, PlaylistHeader_t *h) {
    beginResetModel();
    PlaylistHeader_t *c = columns.at(i);
    DBAPI->tf_free(c->_format_compiled);
    delete c;
    columns.replace(i,h);
    compileFormat(h);
    endResetModel();
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

QString PlayItemModel::formatFromHeaderType(headerType t) {
    // no format for HT_empty, HT_itemIndex, HT_playing and HT_albumArt
    const QString map[] = {"", "", "", "", COLUMN_FORMAT_ARTISTALBUM, COLUMN_FORMAT_ARTIST,
                           COLUMN_FORMAT_ALBUM, COLUMN_FORMAT_TITLE, COLUMN_FORMAT_YEAR, COLUMN_FORMAT_LENGTH,
                           COLUMN_FORMAT_TRACKNUMBER, COLUMN_FORMAT_BAND, COLUMN_FORMAT_CODEC, COLUMN_FORMAT_BITRATE };
    if (t < HT_custom && t > HT_empty) {
        return map[t];
    }
    return "";
}

QString PlayItemModel::titleFromHeaderType(headerType t) {
    #define _(X) dbtr->tr(X)
    const QStringList items = {_("Item Index"), _("♫"), _("Album Art"), _("Artist - Album"),
                         _("Artist"), _("Album"), _("Title"), _("Year"), _("Duration"), _("Track Number"),
                         _("Band / Album Artist"), _("Codec"), _("Bitrate"), _("Custom")};
    #undef _
    if (t < HT_custom+1 && t >= HT_empty+1) {
        return items.at(t-1);
    }
    return "";
}

QList<PlaylistHeader_t *> *PlayItemModel::defaultHeaders() {
    static QList<PlaylistHeader_t *> default_headers;
    default_headers.clear();
    const headerType l_type[] = {HT_playing, HT_artistAlbum, HT_trackNum, HT_title, HT_length, HT_empty};
    // Fill in data
    for (int i = 0; l_type[i] != HT_empty; i++) {
        PlaylistHeader_t *temp = new PlaylistHeader_t;
        temp->title = ""; // to be filled automatically
        temp->type = l_type[i];
        default_headers.append(temp);
    }
    return &default_headers;
}

int PlayItemModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return columns.count();
}

Qt::ItemFlags PlayItemModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (m_playlistLock) {
        return defaultFlags;
    }
    else if (index.isValid()) {
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

QVariant PlayItemModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const {
    QVariant ret;
    if (index.column() > columns.length()-1) {
        return QVariant();
    }
    PlaylistHeader_t *h = columns[index.column()];
    ddb_tf_context_t context;
    {
        context._size = sizeof(ddb_tf_context_t);
        context.flags = 0;
        context.it = track(index);
        if (context.it) {
            context.plt = DBAPI->pl_get_playlist(context.it);
        }
        else {
            context.plt = nullptr;
        }
        context.idx = 0;
        context.id = 0;
        context.iter = m_iter;
        context.update = 0;
        context.dimmed = 0;
    }
    switch (role) {
    case Qt::DisplayRole:
        if (context.it) {
            char buffer[1024]; // TODO hardcoded 1024
            buffer[0] = 0;
            switch (h->type) {
            case HT_itemIndex:
                // TODO start at 1?
                ret = QString::number(index.row());
                break;
            case HT_playing:
                // TODO include information about queue here
                if (property("queueManager").toBool()) {
                    ret = QString("%1") .arg(index.row()+1);
                }
                else if ((DBAPI->playqueue_test(context.it) != -1)) {
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
        }
        break;
    case Qt::DecorationRole:
        if (h->type == HT_playing) {
            DB_playItem_t *curr = DBAPI->streamer_get_playing_track();
            if (curr) {
                if (curr == context.it) {
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
    if (context.it) {
        DBAPI->pl_item_unref(context.it);
    }
    if (context.plt) {
        DBAPI->plt_unref(context.plt);
    }
    return ret;
}

QVariant PlayItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole && section < columns.count()) {
            if (columns[section]->type != headerType::HT_custom && columns[section]->title.isEmpty()) {
                return QVariant(titleFromHeaderType((headerType) columns[section]->type));
            }
            else {
                return QVariant(columns[section]->title);
            }
        }
        else if (role == Qt::SizeHintRole && section < columns.count()) {
            return QVariant(QSize(-1,25));
        }
    }
    return QVariant();
}


void PlayItemModel::onPlaybackChanged() {
    // update all columns with HT_playing
    for (int i = 0; i < columns.count(); i++) {
        if (columns.at(i)->type == HT_playing) {
            emit dataChanged(createIndex(0, i, nullptr), createIndex(trackCount(),i));
        }
    }
}

void PlayItemModel::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    Q_UNUSED(from); Q_UNUSED(to)
    // playback change
    onPlaybackChanged();
}

QStringList PlayItemModel::mimeTypes () const {
    QStringList qstrList = {"deadbeef/playitems"};
    return qstrList;
}

Qt::DropActions PlayItemModel::supportedDropActions () const {
    if (!m_playlistLock)
        return Qt::CopyAction | Qt::MoveAction;
    return Qt::IgnoreAction;
}

QMimeData *PlayItemModel::mimeData(const QModelIndexList &indexes) const {
    playItemList l = tracks(indexes);
    foreach (DB_playItem_t *it, l) {
        DBAPI->pl_item_unref(it);
    }
    return api->mime_playItems(l);
}

bool PlayItemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    if (data->hasFormat("deadbeef/playitems")) {
        return true;
    }
    return false;
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

// Functions to be implemented:

int PlayItemModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 0;
}

playItemList PlayItemModel::tracks(const QModelIndexList &tracks) const {
    Q_UNUSED(tracks)
    return playItemList();
}

playItemList PlayItemModel::tracks(const QList<int> &tracks) const {
    Q_UNUSED(tracks)
    return playItemList();
}

DB_playItem_t * PlayItemModel::track(const QModelIndex &track) const {
    Q_UNUSED(track)
    return nullptr;
}

void PlayItemModel::insertTracks(playItemList *l, int after) {
    Q_UNUSED(l)
    Q_UNUSED(after)
}

void PlayItemModel::moveIndexes(QList<int> l, int after) {
    Q_UNUSED(l)
    Q_UNUSED(after)
}

void PlayItemModel::removeIndexes(QList<int> l) {
    Q_UNUSED(l)
}


int PlayItemModel::trackCount() const {
    return 0;
}

int PlayItemModel::iter() {
    return m_iter;
}

void PlayItemModel::setIter(int iter) {
    beginResetModel();
    m_iter = iter;
    endResetModel();
}
