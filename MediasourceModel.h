#ifndef MEDIASOURCEMODEL_H
#define MEDIASOURCEMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <QFutureWatcher>
#include <QHash>

#include <DBApi.h>
#include "medialib.h"

#define MS_P(X) (static_cast<DB_mediasource_t *>((void *)X))
#define MLP_P(X) (static_cast<ddb_medialib_plugin_t *>((void *)X))

class MediasourceModel : public QAbstractItemModel, public DBWidget {
    Q_OBJECT
public:
    MediasourceModel(QObject *parent = nullptr, DBApi *Api = nullptr, QString plugname = QString());
    ~MediasourceModel();

    DB_mediasource_t * getMediasourcePlugin();
    QStringList getSelectors();

    void setDirectories(QStringList folders);

    static void source_listener(ddb_mediasource_event_type_t event, void *user_data);
    playItemList tracks(QModelIndexList &l);
    playItemList tracks(ddb_medialib_item_t *);
    QModelIndex indexByPath(QStringList &l);

    // Model
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex parent(const QModelIndex &index) const;
signals:
    void listenerCallback();

public slots:
    void setSelector(int selector);
    void setSearchQuery(const QString query);

private slots:
    void onListenerCallback();
    void onCoverReceived();
protected:
    DB_mediasource_t *ms = nullptr;
    ddb_medialib_plugin_t *mlp = nullptr;
    ddb_mediasource_source_t source;
    int listener;

    ddb_mediasource_list_selector_t *selectors_internal = nullptr;
    QStringList selectors_list;
    int selector = 1;
    QString search_query;

    // List
    ddb_medialib_item_t *list = nullptr;
    QMutex *list_mutex;
    // Used for tracks
    QMutex *list_mutex_recursive;
    bool listToBeRefreshed = false;
    bool mediasource_model_reset = false;
    // Hash to map child QModelIndex to parent
    QHash<void *,QModelIndex> *child_to_parent = nullptr;

    // Coverart
    QSet<QImage *> *cover_arts;
    QSet<DB_playItem_t *> *cover_arts_tracks;
    QMutex *cover_arts_lock;
    QHash<QFutureWatcher<QImage *>*, QModelIndex> *future_list;
    QSize cover_size;

    // Folders (if supported)
    QStringList folders;
};

#endif // MEDIASOURCEMODEL_H
