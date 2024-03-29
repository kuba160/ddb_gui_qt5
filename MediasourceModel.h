#ifndef MEDIASOURCEMODEL_H
#define MEDIASOURCEMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <QFutureWatcher>
#include <QHash>

#include <DBApi.h>
#include "medialib.h"

#define MS_P(X) (static_cast<DB_mediasource_t *>((void *)X))
#define MLP_P(X) (static_cast<ddb_medialib_plugin_api_t *>((void *)X))

typedef struct CurrentState_s {
public:
    ddb_medialib_item_t *list = nullptr;
    //QSet<QImage *> cover_arts;
    //QSet<DB_playItem_t *> cover_arts_tracks;
    QHash<DB_playItem_t *, QImage *> cover_arts;
    QHash<DB_playItem_t *, QPixmap> cover_arts_pixmaps;
    //QMutex cover_arts_lock;
    QHash<QFutureWatcher<QImage *>*, QModelIndex> future_list;
    QHash<const void *,QModelIndex> child_to_parent;
    QSet<DB_playItem_t *> cover_enqueued;
} CurrentState_t;

class MediasourceModel : public QAbstractItemModel, public DBWidget {
    Q_OBJECT
public:
    MediasourceModel(QObject *parent = nullptr, DBApi *Api = nullptr, QString plugname = QString());
    ~MediasourceModel();

    QString medialib_name;

    DB_mediasource_t * getMediasourcePlugin();
    ddb_mediasource_state_t getMediasourceState();
    QStringList getSelectors();

    QStringList getDirectories();
    void setDirectories(QStringList folders);

    static void source_listener(ddb_mediasource_event_type_t event, void *user_data);
    playItemList tracks(QModelIndexList &l);
    playItemList tracks(ddb_medialib_item_t *);
    QModelIndex indexByPath(QStringList &l);


    void currentStateClean(CurrentState_t *cs);

    // Model
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &index) const override;
signals:
    // signals emitted from mediasource plugin
    // process them in updateCurrentState/Selectors to not block the plugin
    void mediasource_content_changed();
    void mediasource_selectors_changed();


public slots:
    void updateCurrentState();
    void updateSelectors();


    void setSelector(int selector);
    void setSearchQuery(const QString query);

private slots:
    //void onListenerCallback();
    void onCoverReceived();
protected:
    DB_mediasource_t *ms = nullptr;
    ddb_medialib_plugin_api_t *mlp = nullptr;
    ddb_mediasource_source_t source;
    int listener;

    ddb_mediasource_list_selector_t *selectors_internal = nullptr;
    QStringList selectors_list;
    int selector = 1;
    QString search_query;

    CurrentState_t *cs = nullptr;
    CurrentState_t *cs_old = nullptr;

    QVector<QFutureWatcher<QImage *>*> orphans;
    // List
    //ddb_medialib_item_t *list = nullptr;
    //QMutex *list_mutex;
    // Used for tracks
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    //QRecursiveMutex *list_mutex_recursive;
#else
    //QMutex *list_mutex_recursive;
#endif
    QSize cover_size;

};

#endif // MEDIASOURCEMODEL_H
