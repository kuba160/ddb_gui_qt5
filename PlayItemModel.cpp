#include "PlayItemModel.h"

#include "MainWindow.h"
#include "DeadbeefTranslator.h"

#include "QtGui.h"



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
    #define _(X) dbtr->tr(X)
    const QStringList items = {_("Item Index"), _("♫"), _("Album Art"), _("Artist - Album"),
                     _("Artist"), _("Album"), _("Title"), _("Year"), _("Duration"), _("Track Number"),
                     _("Band / Album Artist"), _("Codec"), _("Bitrate"), _("Custom")};
    if (role >= ItemIndex && role <= LastRoleUnused) {
        return items[role - ItemIndex];
    }
    return QString();
}

PlayItemModel::PlayItemModel(QObject *parent, DBApi *Api) : QAbstractItemModel(parent), DBWidget(nullptr,Api) {
    // Events to update current status next to playing track
    connect(api, SIGNAL(trackChanged()),this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(playbackPaused()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(playbackUnPaused()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(queueChanged()), this, SLOT(onPlaybackChanged()));
    connect(api, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));

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
    }
    DBAPI->tf_eval (&context, format_map.value(role), buffer, 1024);
    return QString(buffer);
}

QVariant PlayItemModel::data(const QModelIndex &index, int role = Qt::DisplayRole) const {
    QVariant ret;
    if (index.isValid()) {
        DB_playItem_t *it = track(index);
        switch (role) {
            case ItemPlayingState:
                {
                    DB_playItem_t *curr = DBAPI->streamer_get_playing_track();
                    if (curr) {
                        if (curr == it) {
                            if (api->isPaused())
                                ret = 2;
                            else
                                ret = 1;
                        }
                        else {
                            ret = 0;
                        }
                        DBAPI->pl_item_unref(curr);
                    }
                }
                break;
            case ItemSelected:
                ret = DBAPI->pl_is_selected(it);
                break;
            case ItemIndex:
                // index guaranteed to be row of this model
                ret = QString::number(index.row());
                break;
            case ItemPlaying:
                // TODO fix queue hack
                if (property("queueManager").toBool()) {
                    ret = QString("%1") .arg(index.row()+1);
                }
                else if ((DBAPI->playqueue_test(it) != -1)) {
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
            case ItemAlbumArt:
                // TODO
                break;
            case Qt::SizeHintRole:
            {
                QSize defSize;
                //TODO: get value from settings
                defSize.setHeight(25);
                ret = defSize;
                break;
            }
            default:
                if (role > Qt::UserRole) {
                    ret = itemTfParse(it, role);
                }
                break;
        }

        if (it) {
            DBAPI->pl_item_unref(it);
        }
    }
    return ret;
}

QVariant PlayItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && section == 0) {
        if (role >= ItemIndex && role < LastRoleUnused) {
            return defaultTitle(role);
        }
        else if (role == Qt::SizeHintRole) {
            return QVariant(QSize(-1,25));
        }
    }
    return QVariant();
}


void PlayItemModel::onPlaybackChanged() {
    emit dataChanged(createIndex(0,0), createIndex(rowCount(),columnCount()), QVector<int>{ItemPlayingState,ItemPlaying});
}

void PlayItemModel::onSelectionChanged() {
    // TODO: better solution?
    emit dataChanged(createIndex(0,0), createIndex(rowCount(), columnCount(QModelIndex())),QVector<int>{ItemSelected});
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
    l.insert(ItemIndex,"ItemIndex");
    l.insert(ItemPlaying,"ItemPlaying");
    l.insert(ItemAlbumArt,"ItemAlbumArt");
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
    l.insert(LastRoleUnused,"LastRoleUnused");
    return l;
}

PlayItemTableModel::PlayItemTableModel(QObject *parent, DBApi *api) : PlayItemModel(parent, api) {
    // get icons
    playIcon = new QIcon(":/root/images/play_16.png");
    pauseIcon = new QIcon(":/root/images/pause_16.png");
}

PlayItemTableModel::~PlayItemTableModel() {
    delete playIcon;
    delete pauseIcon;
}

int PlayItemTableModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return header_titles.length();
}

QByteArray PlayItemTableModel::getHeaderSettings() const {
    QByteArray ba;
    QDataStream ds(&ba,QIODevice::WriteOnly);
    for (int i = 0; i < header_titles.length(); i++) {
        ds << header_titles[i];
        ds << header_roles[i];
        ds << header_format[i];
    }
   return ba;
}

void PlayItemTableModel::setHeaderSettings(QByteArray(ba)) {
    QDataStream ds(&ba,QIODevice::ReadOnly);
    beginResetModel();
    headerm.lock();
    header_titles.clear();
    header_roles.clear();
    header_format.clear();
    headerm.unlock();
    endResetModel();
    QString title;
    int role;
    QString format;
    while (!ds.atEnd()) {
        ds >> title;
        ds >> role;
        ds >> format;
        if (format.isEmpty()) {
            addHeader(role, title);
        }
        else {
            addHeader(title, format);
        }
    }
}

void PlayItemTableModel::setDefaultHeaderSettings() {
    beginResetModel();
    headerm.lock();
    header_titles.clear();
    header_roles.clear();
    header_format.clear();
    headerm.unlock();
    endResetModel();
    addHeader(ItemPlaying,"♫");
    addHeader(ItemArtistAlbum);
    addHeader(ItemTrackNum);
    addHeader(ItemTitle);
    addHeader(ItemLength);
}

int PlayItemTableModel::addHeader(int role, QString title) {
    if (role >= ItemPlaying && role < LastRoleUnused) {
        beginInsertColumns(QModelIndex(), header_titles.length(), header_titles.length());
        headerm.lock();
        header_titles.append(title);
        header_roles.append(role);
        header_format.append("");
        headerm.unlock();
        endInsertColumns();
        return header_titles.length();
    }
    return -1;
}

int PlayItemTableModel::addHeader(QString title, QString format) {
    if (title.isEmpty()) {
        title = "Custom";
    }
    beginInsertColumns(QModelIndex(), header_titles.length(), header_titles.length());
    headerm.lock();
    header_titles.append(title);
    header_roles.append(addFormat(format));
    header_format.append("");
    headerm.unlock();
    endInsertColumns();
    return header_titles.length();
}

void PlayItemTableModel::removeHeader(int num) {
    if (num >= 0 && num < header_titles.length()) {
        beginRemoveColumns(QModelIndex(), num, num);
        headerm.lock();
        header_titles.removeAt(num);
        header_roles.removeAt(num);
        header_format.removeAt(num);
        headerm.unlock();
        endRemoveColumns();
    }
}

void PlayItemTableModel::modifyHeader(int num, int role, QString title, QString format) {
    if (num >= 0 && num < header_titles.length()) {
        if (title != header_titles[num]) {
            header_titles[num] = title;
            emit headerDataChanged(Qt::Horizontal,num,num);
        }
        if (role >= ItemIndex && role < LastRoleUnused) {
            if (header_roles[num] != role) {
                header_roles[num] = role;
                emit dataChanged(createIndex(0,num),createIndex(rowCount(QModelIndex()), num));
            }
        }
        else {
            if (header_format[num].isEmpty() || format != header_format[num] || header_roles[num] != role) {
                header_format[num] = format;
                header_roles[num] = addFormat(format);
                emit dataChanged(createIndex(0,num),createIndex(rowCount(QModelIndex()), num));
            }
        }
    }
}

QVariant PlayItemTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (isValidHeader(section)) {
                if ( header_roles[section] >= ItemIndex && header_roles[section] <= LastRoleUnused) {
                    if (header_titles[section].isEmpty()) {
                        return defaultTitle(header_roles[section]);
                    }
                    else {
                        return header_titles[section];
                    }
                }
            }
        }
        else if (role == Qt::SizeHintRole) {
            return QVariant(QSize(-1,25));
        }
    }
    return QVariant();
}

QVariant PlayItemTableModel::data(const QModelIndex &index, int role) const {
    //qDebug() << "PlayItemTableModel::data, column:" << index.column() << "row:" << index.row();
    if (index.isValid()) {
        if (isValidHeader(index.column())) {
            if (role == Qt::DisplayRole) {
                return PlayItemModel::data(index, header_roles[index.column()]);
            }
            else if (role == Qt::DecorationRole) {
                if (isValidHeader(index.column()) && header_roles[index.column()] == ItemPlaying) {
                    int icon = PlayItemModel::data(index, ItemPlayingState).toInt();
                    if (icon) {
                        return icon == 2 ? *pauseIcon : *playIcon;
                    }
                }
            }
            else {
                return PlayItemModel::data(index, role);
            }
        }
    }
    return QVariant();
}

inline bool PlayItemTableModel::isValidHeader(int num) const {
    if (num >= 0 && num < header_titles.length()) {
        return true;
    }
    return false;
}

QString PlayItemTableModel::getHeaderTitle(int num) {
    if (isValidHeader(num)) {
        return header_titles[num];
    }
    return QString();
}

int PlayItemTableModel::getHeaderRole(int num) {
    if (isValidHeader(num)) {
        return header_roles[num];
    }
    return -1;
}

QString PlayItemTableModel::getHeaderFormat(int num) {
    if (isValidHeader(num)) {
        if (header_roles[num] <= ItemBitrate && header_roles[num] >= ItemArtistAlbum) {
            return defaultFormat(header_roles[num]);
        }
        return header_format[num];
    }
    return QString();
}
