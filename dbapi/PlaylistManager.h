#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <deadbeef/deadbeef.h>

#include "models/PlayItemTableProxyModel.h"
#include "CoverArt.h"

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
    Q_PROPERTY(int current_idx READ getCurrentPlaylistIdx WRITE setCurrentPlaylistIdx NOTIFY currentPlaylistChanged)
    Q_PROPERTY(QAbstractItemModel* current_item READ getCurrentItem CONSTANT)
    Q_PROPERTY(QAbstractItemModel* queue READ getQueue CONSTANT)
    Q_PROPERTY(QAbstractItemModel* list READ getList() CONSTANT);

    // TODO Medialib access

public slots:
    void queueAppend(QVariant mime);


signals:
    void currentPlaylistChanged();

    // Emitted when playback state or queue changes
    void statusRowChanged();

public:
    QAbstractItemModel* getCurrentPlaylist();
    int getCurrentPlaylistIdx();
    void setCurrentPlaylistIdx(int plt);
    QAbstractItemModel* getQueue();
    QAbstractItemModel* getList();
    QAbstractItemModel* getCurrentItem();
    int pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2);


    // standalone functions
    PlayItemTableProxyModel* createTableProxy(QObject *parent);

private:
    PlaylistModel *m_current;
    QAbstractItemModel *m_queue;
    PlaylistBrowserModel *m_list;
    QAbstractItemModel *m_current_item;
};

#endif // PLAYLISTMANAGER_H
