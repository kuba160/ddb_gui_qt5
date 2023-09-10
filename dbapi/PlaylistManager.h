#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <deadbeef/deadbeef.h>

#include "models/PlayItemTableProxyModel.h"
#include "models/PlayItemFilterModel.h"
#include "models/MediasourceModel.h"
#include "MedialibConfig.h"
#include "CoverArt.h"
#include "PlayItemIterator.h"

class PlaylistModel;
class PlaylistBrowserModel;


class PlaylistManager : public CoverArt
{
    Q_OBJECT
public:
    explicit PlaylistManager(QObject *parent, DB_functions_t *api);
    ~PlaylistManager();
    DB_functions_t *deadbeef;

    Q_PROPERTY(QAbstractItemModel* current READ getCurrentPlaylist CONSTANT)
    Q_PROPERTY(QAbstractItemModel* current_search READ getCurrentPlaylistSearch CONSTANT)
    Q_PROPERTY(int current_idx READ getCurrentPlaylistIdx WRITE setCurrentPlaylistIdx NOTIFY currentPlaylistChanged)
    Q_PROPERTY(QAbstractItemModel* current_item READ getCurrentItem CONSTANT)
    Q_PROPERTY(QAbstractItemModel* queue READ getQueue CONSTANT)
    Q_PROPERTY(QAbstractItemModel* list READ getList CONSTANT);

    Q_PROPERTY(QAbstractItemModel* medialib READ getMedialib CONSTANT)
    Q_PROPERTY(QStringList medialib_folders READ getMedialibFolders WRITE setMedialibFolders NOTIFY medialibFoldersChanged)

public slots:
    void queueAppend(QVariant mime);


signals:
    void currentPlaylistChanged();

    // Emitted when playback state or queue changes
    void statusRowChanged();
    void medialibFoldersChanged();

public:
    QAbstractItemModel* getCurrentPlaylist();
    QAbstractItemModel* getCurrentPlaylistSearch();
    int getCurrentPlaylistIdx();
    void setCurrentPlaylistIdx(int plt);
    QAbstractItemModel* getQueue();
    QAbstractItemModel* getList();
    QAbstractItemModel* getCurrentItem();
    QAbstractItemModel* getMedialib();
    QStringList getMedialibFolders();
    void setMedialibFolders(QStringList l);
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);


    // standalone functions
    PlayItemTableProxyModel* createTableProxy(QObject *parent);


    // track insertion
    QFuture<PlayItemIterator> runFileImport(QStringList files);
    QFuture<PlayItemIterator> runFolderImport(QStringList folders);
    QFuture<PlayItemIterator> runPlaylistImport(QStringList playlists);

private:
    PlaylistModel *m_current;
    PlayItemFilterModel *m_current_search;
    QAbstractItemModel *m_queue;
    PlaylistBrowserModel *m_list;
    QAbstractItemModel *m_current_item;
    MediasourceModel *m_medialib = nullptr;
    MedialibConfig *m_medialib_config = nullptr;

};


#endif // PLAYLISTMANAGER_H
