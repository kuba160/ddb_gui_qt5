#include "MediasourceModel.h"
#include <QThread>

#include "medialib.h"

void MediasourceModel::source_listener(ddb_mediasource_event_type_t event, void *user_data) {
    Q_UNUSED(event);
    qDebug() << event;
    if (event == DDB_MEDIASOURCE_EVENT_CONTENT_DID_CHANGE) {
        qDebug() << "State = " << static_cast<MediasourceModel *>(user_data)->getMediasourceState();
        if (static_cast<MediasourceModel *>(user_data)->getMediasourceState() == DDB_MEDIASOURCE_STATE_IDLE) {
            qDebug() << "Updating MEDIA\n";
            emit static_cast<MediasourceModel *>(user_data)->mediasource_content_changed();
        }
        else {
            qDebug() << "Content changed but state is not idle.";
        }
    }
    else if (event == DDB_MEDIASOURCE_EVENT_SELECTORS_DID_CHANGE) {
        qDebug() << "selectors changed";
        emit static_cast<MediasourceModel *>(user_data)->mediasource_selectors_changed();
    }
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
    medialib_name = parent->property("internalName").toString().append("-medialib");
    source = ms->create_source(medialib_name.toUtf8());
    connect(this, SIGNAL(mediasource_content_changed()), this, SLOT(updateCurrentState()));
    connect(this, SIGNAL(mediasource_content_changed()), this, SLOT(updateSelectors()));
    listener = ms->add_listener(source,source_listener,this);

    updateSelectors();

    size_t folsize;
    char **fout = mlp->get_folders(source, &folsize);
    size_t i = 0;
    while (fout && i < folsize) {
        folders.append(fout[i]);
    }


    cover_size = QSize(24,24);

    ms->set_source_enabled(source, true);
    ms->refresh(source);
}

MediasourceModel::~MediasourceModel() {
    ms->remove_listener(source,listener);
/*
    while (future_list->count()) {
        QThread::msleep(10);
    }

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
        ms->free_item_tree(source,list);
    }
    ms->free_selectors_list(source,selectors_internal);
    ms->free_source(source);

    delete list_mutex;
    delete list_mutex_recursive;
    delete cover_arts;
    delete cover_arts_tracks;
    delete cover_arts_lock;
    delete child_to_parent;
    delete future_list;
    */
}

void MediasourceModel::updateCurrentState() {
    beginResetModel();
    if (cs_old) {
       // cs->list_mutex->lock();
        //list_mutex_recursive->lock();
        if (cs_old->list) {
            ms->free_item_tree(source,cs->list);
            cs_old->child_to_parent.clear();
        }
        // CoverArt free
        //cover_arts_lock->lock();
        QSet<QImage *>::const_iterator i = cs_old->cover_arts.constBegin();
        while (i != cs_old->cover_arts.constEnd()) {
            api->coverArt_unref(*i);
            ++i;
        }
        QSet<DB_playItem_t *>::const_iterator j = cs_old->cover_arts_tracks.constBegin();
        while (j != cs->cover_arts_tracks.constEnd()) {
            api->coverArt_track_unref(*j);
            ++j;
        }
        cs_old->cover_arts.clear();
        cs_old->cover_arts_tracks.clear();
        cs_old->child_to_parent.clear();
        while(cs_old->future_list.count()) {
            // wait for existing artwork calls (bad code)
            QThread::usleep(1000);
        }
        cs_old->future_list.clear();
        delete cs_old;
        cs_old = nullptr;
    }

    cs_old = cs;

    // create new "current state", cache covers and replace the old one TODO???
    cs = new CurrentState_t;

    // update folders
    QVector<const char*> vec;
    if (folders.length() > 0) {
        foreach(QString str, folders) {
            vec.append(strdup(str.toUtf8().constData()));
            qDebug() << str;
        }
    }
    else {
        vec.append(nullptr);
    }
    if (mlp) {
        mlp->set_folders(source,vec.data(),folders.length() ? folders.length() : 1);
    }
    if (folders.length()) {
        for (int i = 0; i < folders.length(); i++) {
            free((void *) vec[i]);
        }
    }

    cs->list = ms->create_item_tree(source,selectors_internal[selector], search_query.isEmpty() ? " " : search_query.toUtf8());

    endResetModel();
}

void MediasourceModel::updateSelectors() {
    if (selectors_internal) {
        selectors_list.clear();
        ms->free_selectors_list(source,selectors_internal);
    }
    selectors_internal = ms->get_selectors_list(source);

    for (int i = 0; 1; i++) {
        const char* s = ms->selector_name(source,selectors_internal[i]);
        if (s) {
            selectors_list.append(s);
        }
        else {
            break;
        }
    }
}

DB_mediasource_t *MediasourceModel::getMediasourcePlugin() {
    return ms;
}

ddb_mediasource_state_t MediasourceModel::getMediasourceState() {
    return ms->scanner_state(source);
}
/*
void MediasourceModel::onListenerCallback() {
    if(future_list->count()) {
        listToBeRefreshed = true;
        return;
    }
    beginResetModel();
    list_mutex->lock();
    list_mutex_recursive->lock();
    if (list) {
        ms->free_item_tree(source,list);
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
        ms->free_selectors_list(source,selectors_internal);
        ms->remove_listener(source,listener);
        ms->free_source(source);
        //
        source = ms->create_source("medialib");
        listener = ms->add_listener(source,source_listener,this);
        selectors_internal = ms->get_selectors_list(source);
        ms->refresh(source);
    }
    // List update
    listToBeRefreshed = false;
    list = ms->create_item_tree(source,selectors_internal[selector], search_query.isEmpty() ? " " : search_query.toUtf8());

    // Full source reset (cont.)
    if (mediasource_model_reset) {
        QVector<const char*> vec;
        if (folders.length() > 0) {
            foreach(QString str, folders) {
                vec.append(strdup(str.toUtf8().constData()));
                qDebug() << str;
            }
        }
        else {
            vec.append(nullptr);
        }
        if (mlp) {
            mlp->set_folders(source,vec.data(),folders.length() ? folders.length() : 1);
        }
        if (folders.length()) {
            for (int i = 0; i < folders.length(); i++) {
                free((void *) vec[i]);
            }
        }
        mediasource_model_reset = false;
    }
    cover_arts_lock->unlock();
    list_mutex_recursive->unlock();
    list_mutex->unlock();
    endResetModel();
}*/

void MediasourceModel::onCoverReceived() {
    QFutureWatcher<QImage *> *emitter = static_cast<QFutureWatcher<QImage *>*>(sender());
    CurrentState_t *state = nullptr;
    if (cs->future_list.contains(emitter)) {
        state = cs;
    }
    else if (cs_old->future_list.contains(emitter)) {
        state = cs_old;
    }
    if (state) {
        //cover_arts_lock->lock();
        QModelIndex index = state->future_list.take(emitter);
        QImage *img = emitter->result();
        if (img && index.isValid() && index.internalPointer()) {
            ddb_medialib_item_t *root = static_cast<ddb_medialib_item_t *>(index.internalPointer());
            if (root->children && root->children->track) {
                // cover used, put in set to be unrefed later
                if (state->cover_arts.contains(img)) {
                    // fix for multiple cover art searches
                    qDebug() << "cover already in current state cache?";
                    api->coverArt_unref(img);
                }
                state->cover_arts.insert(emitter->result());
                state->cover_arts_tracks.insert(root->children->track);
            }
        }
        //cover_arts_lock->unlock();
        // refresh index where cover art should be
        if (state == cs && img && index.isValid() && index.internalPointer()) {
            emit dataChanged(index,index,QVector<int>(1,Qt::DecorationRole));
        }
        delete emitter;
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
            cs->child_to_parent.insert(child,parent);
            return createIndex(row,column,child);
        }
    }
    if (row == 0 && column == 0 && !parent.isValid()) {
        // create root
        return createIndex(row, column, cs->list);
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
    //list_mutex->lock();
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
                //cover_arts_lock->lock();
                if (api->isCoverArtCached(playit)) {
                    QImage *img_orig = api->getCoverArt(playit);
                    if (img_orig) {
                        if (cs->cover_arts.contains(img_orig)) {
                            api->coverArt_unref(img_orig);
                        }
                        else {
                            cs->cover_arts.insert(img_orig);
                            cs->cover_arts_tracks.insert(playit);
                        }
                        QImage *img = api->getCoverArtScaled(img_orig,cover_size);
                        ret = QPixmap::fromImage(*img);
                    }
                }
                else if (!cs->future_list.values().contains(index)) {
                    QFutureWatcher<QImage *> *fw = new QFutureWatcher<QImage *>;
                    connect(fw, SIGNAL(finished()), this, SLOT(onCoverReceived()));
                    cs->future_list.insert(fw,index);
                    fw->setFuture(api->requestCoverArt(playit,cover_size));
                }
                //cover_arts_lock->unlock();
            }
        }
    }
    }
    //list_mutex->unlock();
    return ret;
}

QModelIndex MediasourceModel::parent(const QModelIndex &index) const {
    if (index.isValid() && index.internalPointer()) {
        ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(index.internalPointer());
        if (cs->child_to_parent.contains(it)) {
            return cs->child_to_parent.value(it);
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
    //list_mutex_recursive->lock();
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
    //list_mutex_recursive->unlock();
    return pil;
}

void MediasourceModel::setSelector(int sel) {
    if (sel < selectors_list.length()) {
        selector = sel;
        updateCurrentState();
        //onListenerCallback();
    }
}

QStringList MediasourceModel::getSelectors() {
    return selectors_list;
}

void MediasourceModel::setSearchQuery(const QString str) {
    search_query = str;
    updateCurrentState();
    //onListenerCallback();
}

void MediasourceModel::setDirectories(QStringList folders_inc) {
    if (!mlp) {
        qDebug() << "MediasourceModel: setDirectories called but not supported on this model!";
        return;
    }
    folders = folders_inc;
    int i = 0;
    while (i < folders_inc.length()) {
        mlp->append_folder(source, folders_inc[i++].toUtf8());
    }
    //mediasource_model_reset = true;
    ms->refresh(source);
    //updateCurrentState();
    //onListenerCallback();
}

QModelIndex MediasourceModel::indexByPath(QStringList &l) {
    int i = 0;
    QModelIndex idx = QModelIndex();
    ddb_medialib_item_t *it = cs->list;
    int row = 0;
    while (it && i < l.length()) {
        if (l[i] == QString(it->text)) {
            idx = index(row,0,idx);
            if (i+1 < l.length()) {
                it = it->children;
                row = 0;
                i++;
            }
            else {
                break;
            }
        }
        else {
            it = it->next;
            row++;
        }
    }
    if (it && l.last() == QString(it->text)) {
        return idx;
    }
    return QModelIndex();
}
