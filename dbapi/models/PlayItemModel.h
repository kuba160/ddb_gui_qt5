#ifndef PLAYITEMMODEL_H
#define PLAYITEMMODEL_H

#include <QAbstractItemModel>
#include <QMutex>
#include <QFutureWatcher>
#include <QMimeData>
#include <deadbeef/deadbeef.h>

#include "../PlaylistManager.h"

typedef QList<DB_playItem_t *> playItemList;

QDataStream &operator<<(QDataStream &ds, const playItemList &pil);
QDataStream &operator>>(QDataStream &ds, playItemList &pil);

class PlayItemMimeData : public QMimeData {
    Q_OBJECT
    DB_functions_t *api;
public:
    PlayItemMimeData(DB_functions_t *ddb, QList<DB_playItem_t*>);
    ~PlayItemMimeData();
    QList<DB_playItem_t*> getTracks() const;

};

class PlayItemModel : public QAbstractItemModel {
    Q_OBJECT
protected:
    PlaylistManager *manager;
public:
    enum playItemRoles {
        ItemEmpty = Qt::UserRole,
        ItemMime,
        ItemActionContext,
        ItemCursor,
        ItemQueue,
        ItemNumber, // equvalent to ItemIndex+1
        //PlayItemRoleFirst = Qt::UserRole,
        ItemAlbumArtUrl,
        PlayItemRoleFirst = Qt::UserRole +32,
        //ItemEmpty = PlayItemRoleFirst,
        ItemPlayingState, // (0 - none, 1 - playing icon, 2 - paused icon)
        ItemPlayingDisplay,
        ItemPlayingDecoration, // does nothing but can be overriden
        ItemSelected,
        ItemIndex, // First "selectable" role
        ItemPlaying,
        ItemAlbumArt, // First role with format
        ItemArtistAlbum,
        ItemArtist,
        ItemAlbum,
        ItemTitle,
        ItemYear,
        ItemLength,
        ItemTrackNum,
        ItemBandAlbumArtist,
        ItemCodec,
        ItemBitrate,
        PlayItemRoleLast = ItemBitrate,
        CustomRoleStart = 0x0200
    };
    Q_ENUM(playItemRoles)
    PlayItemModel(QObject *parent, PlaylistManager *manager);
    ~PlayItemModel();

    /// FUNCTIONS TO BE IMPLEMENTED
    /// (playItem provider)

    // rowCount(&QModelIndex): return row count for given index
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override = 0;

    // tracks(QList<int>): return playItemList for given indices
    // NOTE: tracks have to be unref'd by receiver after use
    virtual playItemList tracks(const QList<int> &tracks) const = 0;

    // insertTracks(playItemList, int): insert playItems after given index
    virtual void insertTracks(playItemList *l, int after);

    // moveIndexes(QList<int> indices, int): move given indices to be after given index
    // TODO: this can be implemented with Qt function overrides
    virtual void moveIndexes(QList<int> indices, int after);

    // removeIndexes(QList<int>): remove given indices
    // TODO: this can be implemented with Qt function overrides
    virtual void removeIndexes(QList<int> indices);

    // sort(int, Qt::SortOrder): optional
    //void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    /// FUNCTIONS THAT CAN BE IMPLEMENTED
    /// (Column provider)

    // columnCount(&QModelIndex): return column count, default is 1
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // headerData(int, Qt::Orientation, int): return column data
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // data(&QModelIndex, int): reinterpret/rearrange data
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /// ACCESIBLE FUNCTIONS THAT
    /// CAN BE USED

    // addFormat(format): Requests format to be added to model
    // It will be available during the lifetime of the model
    // Returns: role for requested format
    int addFormat(QString format);

    // tracks(QModelIndexList): returns playItemList for given indices
    // WRAPPER for convenience, uses playItemProvider
    inline virtual playItemList tracks(const QModelIndexList &index_list) const {
        QList<int> l;
        foreach(QModelIndex idx, index_list) {
            if (!l.contains(idx.row())) {
                l.append(idx.row());
            }
        }
        return tracks(QList<int>{l});
    }

    // track(QModelIndex): returns playItem for given index
    // WRAPPER for convenience, uses playItemProvider
    inline virtual DB_playItem_t *track(const QModelIndex &track) const {
        playItemList l = tracks(QList<int>{track.row()});
        if (l.length()) {
            return l[0];
        }
        return nullptr;
    }

    // roleNames(): returns role-to-string map
    // USED for Qt Quick
    QHash<int, QByteArray> roleNames() const override;

    // defaultFormat(int): returns default format for given role
    // VALID for roles ranging from ItemArtistAlbum to ItemBitrate
    static const QString defaultFormat(int role);

    // defaultTitle(int): returns default title for given role
    // VALID for roles raning from ItemIndex to LastRoleUnused
    static const QString defaultTitle(int role);

    // itemTfParse(playItem, int): parse given playItem with specific role
    // NOTE: used internally, probably not needed to access/modify
    QString itemTfParse(DB_playItem_t *it, int role) const;

    /// PROPERTIES
    /// CAN BE CHANGED

    // iter: PL_MAIN (default) or PL_SEARCH
    Q_PROPERTY(int iter READ getIter WRITE setIter NOTIFY iterChanged);
    int getIter() const;
    void setIter(int);

    // playlist_lock: prevent from editinIg/modifying the model
    Q_PROPERTY(bool playlist_lock READ getPlaylistLock WRITE setPlaylistLock NOTIFY playlistLockChanged);
    bool getPlaylistLock() const;
    void setPlaylistLock(bool);

signals:
    void iterChanged();
    void playlistLockChanged();

public:
    /// FUNCTIONS THAT
    /// CAN BE REIMPLEMENTED

    // index(int, int, &QModelIndex): see Qt docs
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    // parent(&QModelIndex): see Qt docs
    QModelIndex parent(const QModelIndex &child) const override;

    // flags(&QModelIndex): see Qt docs
    // DEFAULT: Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled (Qt::ItemIsDropEnabled for invalid index)
    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    // Property private
    bool m_playlistLock = false;
    int m_iter = PL_MAIN;

private:
    qint64 source_id;

    // Copy/paste
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions () const override;


    // custom format-to-role mapping (one way)
    // "%format%" => Qt::Role => tf_compiled
    QHash<QString, int> format_role_map;
    QHash<int, char *> format_map;

    // last unused role, needed for addFormat()
    int custom_role_last = CustomRoleStart;

private slots:
    void onPlaybackChanged();
    void onSelectionChanged();
    void onCoverArtChanged(int type, int source_id, qint64 userdata);
};

class CurrentPlayItemModel : public PlayItemModel {
    Q_OBJECT
public:
    CurrentPlayItemModel(QObject *parent, PlaylistManager *Manager);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    playItemList tracks(const QList<int> &tracks) const override;
private slots:
    void onPlaybackChanged();
};

#endif // PLAYITEMMODEL_H
