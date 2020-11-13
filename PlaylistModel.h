#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QMimeData>

#include "DBApi.h"

enum headerType{
    HT_empty = 0,
    HT_itemIndex,
    HT_playing,
    HT_albumArt,
    HT_artistAlbum,
    HT_artist,
    HT_album,
    HT_title,
    HT_year,
    HT_length,
    HT_trackNum,
    HT_bandAlbumArtist,
    HT_codec,
    HT_bitrate,
    HT_custom
};

typedef struct PlaylistHeader_s {
    QString title;
    headerType type;
    // Format if type is HT_custom
    QString format = "";
    // In PlaylistModel - compiled format
    char *_format_compiled = nullptr;
} PlaylistHeader_t;

QDataStream &operator<<(QDataStream &ds, const PlaylistHeader_t &pil);
QDataStream &operator>>(QDataStream &ds, PlaylistHeader_t &pil);

class PlaylistModel : public QAbstractItemModel, public DBWidget {
    Q_OBJECT
    
public:
    PlaylistModel(QObject *parent = nullptr, DBApi *Api = nullptr);
    ~PlaylistModel();
    void setColumns(QList<PlaylistHeader_t *> &columns);
    void setPlaylist(ddb_playlist_t *plt);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;

    void insertByURLAtPosition(const QUrl &url, int position = -1);
    void insertByPlayItemAtPosition(DB_playItem_t *item, int position = -1);
    void moveItems(QList<int> indices, int before);
    
    void clearPlayList();
    void deleteTracks(const QModelIndexList &tracks);

    QList<PlaylistHeader_t *> columns;

    //
    QList<PlaylistHeader_t *> *setDefaultHeaders();
    static QString formatFromHeaderType(headerType);
    static QString titleFromHeaderType(headerType);

private:
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    

    int rowCount(const QModelIndex &parent) const;
    
    QModelIndex parent(const QModelIndex &child) const;
    
    Qt::DropActions supportedDropActions () const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

protected:
    ddb_playlist_t *plt = nullptr;

    QIcon playIcon;
    QIcon pauseIcon;

public slots:
    void onPlaybackChanged();
    void onTrackChanged(DB_playItem_t *from, DB_playItem_t *to);

signals:
    void columnsChanged();
};

#endif // PLAYLISTMODEL_H
