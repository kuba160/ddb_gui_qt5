#include "MediasourceModel.h"

#include "medialib.h"

void MediasourceModel::source_listener(ddb_mediasource_event_type_t event, void *user_data) {
    Q_UNUSED(event);
    emit static_cast<MediasourceModel *>(user_data)->listenerCallback();
}

MediasourceModel::MediasourceModel(QObject *parent, DBApi *Api, QString plugname) : QAbstractItemModel(parent), DBWidget(nullptr,Api) {
    ms = MS_P(api->deadbeef->plug_get_for_id(plugname.toUtf8()));
    if (ms && plugname == "medialib") {
        mlp = MLP_P(ms);
    }
    if (ms->plugin.type != DB_PLUGIN_MEDIASOURCE) {
        qDebug() << "plugin " << plugname << " is not a mediasource!";
        return;
    }
    source = ms->create_source(parent->property("internalName").toString().append("-medialib").toUtf8());

    connect(this, SIGNAL(listenerCallback()), this, SLOT(onListenerCallback()));
    listener = ms->add_listener(source,source_listener,this);

    selectors_internal = ms->get_selectors(source);

    for (int i = 0; 1; i++) {
        const char* s = ms->get_name_for_selector(source,selectors_internal[i]);
        if (s) {
            selectors_list.append(s);
        }
        else {
            break;
        }
    }

    // Cover
    cover_arts = new QSet<QImage *>();
    cover_arts_tracks = new QSet<DB_playItem_t *>();
    cover_size = QSize(24,24);
    cover_arts_lock = new QMutex();
    future_list = new QHash<QFutureWatcher<QImage *>*, QModelIndex>;
    list_mutex = new QMutex();
    list_mutex_recursive = new QRecursiveMutex();

    child_to_parent = new QHash<void*,QModelIndex>();
    //list = ms->create_list(source,selectors_internal[selector],search_query.toUtf8());
}

MediasourceModel::~MediasourceModel() {
    cover_arts_lock->lock();
    QSet<QImage *>::const_iterator i = cover_arts->constBegin();
    while (i != cover_arts->constEnd()) {
        api->coverArt_unref(*i);
        ++i;
    }
    cover_arts->clear();
    cover_arts_lock->unlock();
    if (list) {
        // cannot free list here? todo
        //ms->free_list(source,list);
    }
    ms->free_selectors(source,selectors_internal);
    ms->remove_listener(source,listener);
    ms->free_source(source);

    delete list;
    delete list_mutex;
    delete list_mutex_recursive;
    delete cover_arts;
    delete cover_arts_tracks;
    delete cover_arts_lock;
    delete future_list;
}

DB_mediasource_t *MediasourceModel::getMediasourcePlugin() {
    return ms;
}

void MediasourceModel::onListenerCallback() {
    if(future_list->count()) {
        listToBeRefreshed = true;
        return;
    }
    beginResetModel();
    list_mutex->lock();
    list_mutex_recursive->lock();
    if (list) {
        ms->free_list(source,list);
        child_to_parent->clear();
    }
    // CoverArt free
    cover_arts_lock->lock();
    QSet<QImage *>::const_iterator i = cover_arts->constBegin();
    while (i != cover_arts->constEnd()) {
        api->coverArt_unref(*i);
        ++i;
    }
    QSet<DB_playItem_t *>::const_iterator j = cover_arts_tracks->constBegin();
    while (j != cover_arts_tracks->constEnd()) {
        api->coverArt_track_unref(*j);
        ++j;
    }
    cover_arts->clear();
    cover_arts_tracks->clear();

    // Full source reset (when updating folders)
    if (mediasource_model_reset) {
        ms->free_selectors(source,selectors_internal);
        ms->remove_listener(source,listener);
        ms->free_source(source);
        //
        source = ms->create_source("medialib");
        listener = ms->add_listener(source,source_listener,this);
        selectors_internal = ms->get_selectors(source);
    }
    // List update
    listToBeRefreshed = false;
    list = ms->create_list(source,selectors_internal[selector], search_query.isEmpty() ? " " : search_query.toUtf8());

    // Full source reset (cont.)
    if (mediasource_model_reset) {
        const char *arr[folders.length() ? folders.length() : 1];
        if (folders.length() > 0) {
            int i = 0;
            foreach(QString str, folders) {
                arr[i] = strdup(str.toUtf8().constData());
                i++;
            }
        }
        else {
            arr[0] = nullptr;
        }
        if (mlp) {
            mlp->set_folders(source,arr,folders.length() ? folders.length() : 1);
        }
        if (folders.length()) {
            for (int i = 0; i < folders.length(); i++) {
                free((void *) arr[i]);
            }
        }
        mediasource_model_reset = false;
    }
    cover_arts_lock->unlock();
    list_mutex_recursive->unlock();
    list_mutex->unlock();
    endResetModel();
}

void MediasourceModel::onCoverReceived() {
    QFutureWatcher<QImage *> *emitter = static_cast<QFutureWatcher<QImage *>*>(sender());
    if (future_list->contains(emitter)) {
        cover_arts_lock->lock();
        QModelIndex index = future_list->take(emitter);
        QImage *img = emitter->result();
        if (img && index.isValid() && index.internalPointer()) {
            ddb_medialib_item_t *root = static_cast<ddb_medialib_item_t *>(index.internalPointer());
            if (root->children && root->children->track) {
                // cover used, put in set to be unrefed later
                if (cover_arts->contains(img)) {
                    api->coverArt_unref(img);
                }
                cover_arts->insert(emitter->result());
                cover_arts_tracks->insert(root->children->track);
            }
        }
        cover_arts_lock->unlock();
        // refresh index where cover art should be
        if (img && index.isValid() && index.internalPointer()) {
            QVector<int> roles;
            roles.append(Qt::DecorationRole);
            emit dataChanged(index,index,roles);
        }
        delete emitter;
        // refresh if all covers received and the list has changed
        if (listToBeRefreshed && !future_list->count()) {
            onListenerCallback();
        }
    }
    else {
        qDebug() << "invalid cover receive!";
    }
}

QModelIndex MediasourceModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && column == 0) {
        // go into specific item and create model index
        if (parent.isValid() && parent.internalPointer()) {
            ddb_medialib_item_t *root = static_cast<ddb_medialib_item_t *>(parent.internalPointer());
            ddb_medialib_item_t *child = root->children;
            int i = row;
            while (i) {
                if (!child->next) {
                    qDebug() << "too much!";
                    break;
                }
                child = child->next;
                i--;
            }
            child_to_parent->insert(child,parent);
            return createIndex(row,column,child);
        }
    }
    if (row == 0 && column == 0 && !parent.isValid()) {
        // create root
        return createIndex(row, column, list);
    }
    return QModelIndex();
}

int MediasourceModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid() && parent.internalPointer()) {
        ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(parent.internalPointer());
        return it->num_children;
    }
    return 1;
}

int MediasourceModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 1;
    }
    return 1;
}

QVariant MediasourceModel::data(const QModelIndex &index, int role) const {
    QVariant ret;
    list_mutex->lock();
    switch(role) {
    case Qt::DisplayRole: {
        if (ms->scanner_state(source) != DDB_MEDIASOURCE_STATE_IDLE && !index.parent().isValid()) {
            const char *state_str[] = {nullptr, "loading", "scanning", "indexing", "saving"};
            ret = QString("Medialib is %1...") .arg(state_str[ms->scanner_state(source)]);
            break;
        }
        if (index.isValid() && index.internalPointer()) {
            ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(index.internalPointer());
            ret = it->text;
        }
        break;
    }
    case Qt::DecorationRole: {
        if (index.isValid() && index.internalPointer()) {
            ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(index.internalPointer());
            DB_playItem_t *playit = it->children ? it->children->track : nullptr;
            if (playit && api->isCoverArtPluginAvailable()) {
                cover_arts_lock->lock();
                if (api->isCoverArtCached(playit)) {
                    QImage *img_orig = api->getCoverArt(playit);
                    if (img_orig) {
                        if (cover_arts->contains(img_orig)) {
                            api->coverArt_unref(img_orig);
                        }
                        else {
                            cover_arts->insert(img_orig);
                            cover_arts_tracks->insert(playit);
                        }
                        QImage *img = api->getCoverArtScaled(img_orig,cover_size);
                        ret = QPixmap::fromImage(*img);
                    }
                }
                else if (!future_list->values().contains(index) && !listToBeRefreshed) {
                    QFutureWatcher<QImage *> *fw = new QFutureWatcher<QImage *>;
                    connect(fw, SIGNAL(finished()), this, SLOT(onCoverReceived()));
                    future_list->insert(fw,index);
                    fw->setFuture(api->requestCoverArt(playit));
                }
                cover_arts_lock->unlock();
            }
        }
    }
    }
    list_mutex->unlock();
    return ret;
}

QModelIndex MediasourceModel::parent(const QModelIndex &index) const {
    if (index.isValid() && index.internalPointer()) {
        ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(index.internalPointer());
        if (child_to_parent->contains(it)) {
            return child_to_parent->value(it);
        }
    }
    return QModelIndex();
}

playItemList MediasourceModel::tracks(QModelIndexList &l) {
    playItemList pil;
    foreach(QModelIndex idx, l) {
        if (idx.isValid() && idx.internalPointer()) {
            ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(idx.internalPointer());
            playItemList pil_child = tracks(it);
            foreach(DB_playItem_t *it, pil_child) {
                if (!pil.contains(it)) {
                    pil.append(it);
                }
            }
        }
    }
    return pil;
}

playItemList MediasourceModel::tracks(ddb_medialib_item_t *l) {
    list_mutex_recursive->lock();
    playItemList pil;
    ddb_medialib_item_t *child = l->children;
    while (child) {
        playItemList pil_child = tracks(child);
        foreach(DB_playItem_t *it, pil_child) {
            if (!pil.contains(it)) {
                pil.append(it);
                DBAPI->pl_item_ref(it);
            }
        }
        child = child->next;
    }
    if (l->track && !pil.contains(l->track)) {
        DBAPI->pl_item_ref(l->track);
        pil.append(l->track);
    }
    list_mutex_recursive->unlock();
    return pil;
}

void MediasourceModel::setSelector(int sel) {
    if (sel < selectors_list.length()) {
        selector = sel;
        onListenerCallback();
    }
}

QStringList MediasourceModel::getSelectors() {
    return selectors_list;
}

void MediasourceModel::setSearchQuery(const QString str) {
    search_query = str;
    onListenerCallback();
}

void MediasourceModel::setDirectories(QStringList folders_inc) {
    if (!mlp) {
        qDebug() << "MediasourceModel: setDirectories called but not supported on this model!";
        return;
    }
    folders = folders_inc;
    mediasource_model_reset = true;
    onListenerCallback();
}
