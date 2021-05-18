#ifndef PLAYITEMMODEL_H
#define PLAYITEMMODEL_H

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
    HT_custom,
    HT_END
};

typedef struct PlaylistHeader_s {
    QString title;
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    headerType type;
#else
    unsigned int type;
#endif
    // Format if type is HT_custom
    QString format = "";
    // compiled format, to be done by PlaylistModel
    char *_format_compiled = nullptr;
} PlaylistHeader_t;

QDataStream &operator<<(QDataStream &ds, const PlaylistHeader_t &pil);
QDataStream &operator>>(QDataStream &ds, PlaylistHeader_t &pil);

class PlayItemModel : public QAbstractItemModel, public DBWidget {
    Q_OBJECT

public:
    PlayItemModel(QObject *parent = nullptr, DBApi *api_a = nullptr);
    ~PlayItemModel();

    /// TO BE IMPLEMENTED
    // returns tracks to use for something, remember to unref after use
    virtual playItemList tracks(const QModelIndexList &tracks) const;
    virtual playItemList tracks(const QList<int> &tracks) const;
    // same as above, but for one track
    virtual DB_playItem_t *track(const QModelIndex &track) const;
    // insert track
    virtual void insertTracks(playItemList *l, int after);
    // move indexes
    virtual void moveIndexes(QList<int> indices, int after);
    // remove tracks
    virtual void removeIndexes(QList<int> indices);
    // number of rows
    virtual int rowCount(const QModelIndex &parent) const;
    // same as above currently? :(
    virtual int trackCount() const;
    // sort function
    // use default sort reimplementation

    /// END OF FUNCTIONS TO BE IMPLEMENTED
    //
    /// Static functions
    // returns default column setup
    static QList<PlaylistHeader_t *> *defaultHeaders();
    // returns default format for specific header type
    static QString formatFromHeaderType(headerType);
    // returns default title for specific header type
    static QString titleFromHeaderType(headerType);


    // Functions that can be reimplemented:
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    int columnCount(const QModelIndex &parent) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    // Column manipulation functions
public:
    // make model follow specific column definition, takes ownership of columns data
    void setColumns(QList<PlaylistHeader_t *> *columns = nullptr);
    void addColumn(PlaylistHeader_t *, int before = -1);
    void replaceColumn(int i, PlaylistHeader_t *);
    void removeColumn(int i);

protected:
    // column data, do not modify
    QList<PlaylistHeader_t *> columns;
    // Icons for use to render
    QIcon playIcon;
    QIcon pauseIcon;




private:
    // compile format for playlist header
    void compileFormat(PlaylistHeader_t *h);
    // Copy/paste
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const;
    Qt::DropActions supportedDropActions () const;


protected:
    bool m_playlistLock = false;
    // Properties to set up
    Q_PROPERTY(bool m_playlistLock READ playlistLock WRITE setPlaylistLock NOTIFY playlistLockChanged)
public:
    bool playlistLock() const {return m_playlistLock;}
    void setPlaylistLock(bool lock) {m_playlistLock = lock;emit playlistLockChanged();}

private slots:
    void onPlaybackChanged();
    void onTrackChanged(DB_playItem_t *from, DB_playItem_t *to);

signals:
    void columnsChanged();
    void rowsChanged();
    void playlistLockChanged();
};

#endif // PLAYITEMMODEL_H
