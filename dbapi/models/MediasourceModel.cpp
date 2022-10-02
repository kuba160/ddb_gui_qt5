#include "MediasourceModel.h"
#include <QThread>

#include "medialib.h"

#define DBAPI (manager->deadbeef)

void MediasourceModel::source_listener(ddb_mediasource_event_type_t event, void *user_data) {
    Q_UNUSED(event);
    qDebug() << "source_listener: event" << event;
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
    else if (event == DDB_MEDIASOURCE_EVENT_STATE_DID_CHANGE) {
        //emit static_cast<MediasourceModel *>(user_data)->mediasource_content_changed();
    }
}

MediasourceModel::MediasourceModel(QObject *parent, PlaylistManager *Manager, QString plugname) : QAbstractItemModel(parent) {
    manager = Manager;
    ms = MS_P(DBAPI->plug_get_for_id(plugname.toUtf8()));
    if (ms && plugname == "medialib") {
        mlp = MLP_P(ms->get_extended_api());
    }
    if (ms->plugin.type != DB_PLUGIN_MEDIASOURCE) {
        qDebug() << "plugin " << plugname << " is not a mediasource!";
        return;
    }
    medialib_name = "deadbeef";//parent->property("internalName").toString().append("-qt5");
    source = ms->create_source(medialib_name.toUtf8().constData());

    connect(this, &MediasourceModel::mediasource_content_changed,
            this, &MediasourceModel::updateCurrentState);
    //connect(this, SIGNAL(mediasource_content_changed()), this, SLOT(updateSelectors()));
    listener = ms->add_listener(source,source_listener,this);

    updateSelectors();

    if (!ms->is_source_enabled(source)) {
        ms->set_source_enabled(source, true);
    }

    qDebug() << getDirectories();

}

MediasourceModel::~MediasourceModel() {
    if (cs) {
        currentStateClean(cs);
    }
    if (cs_old) {
        currentStateClean(cs_old);
    }
    ms->remove_listener(source,listener);
    ms->free_source(source);
}

void MediasourceModel::currentStateClean(CurrentState_t *curr_state) {
    if (curr_state->list) {
        ms->free_item_tree(source,curr_state->list);
        curr_state->child_to_parent.clear();
    }

    delete curr_state;
}

void MediasourceModel::updateCurrentState() {
    if (getMediasourceState() != DDB_MEDIASOURCE_STATE_IDLE) {
        emit dataChanged(createIndex(0,0,nullptr),createIndex(0,0,nullptr));
        return;
    }
    beginResetModel();
    if (cs_old) {
        currentStateClean(cs_old);
        cs_old = nullptr;
    }

    cs_old = cs;

    // create new "current state", cache covers and replace the old one TODO???
    cs = new CurrentState_t;

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

QModelIndex MediasourceModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && column == 0) {
        // go into specific item and create model index
        if (parent.isValid() && parent.internalPointer()) {
            ddb_medialib_item_t *root = static_cast<ddb_medialib_item_t *>(parent.internalPointer());
            const ddb_medialib_item_t *child = ms->tree_item_get_children(root);
            int i = row;
            while (i) {
                const ddb_medialib_item_t *child_new = ms->tree_item_get_next(child);
                if (!child_new) {
                    qDebug() << "too much!";
                    break;
                }
                child = child_new;
                i--;
            }
            cs->child_to_parent.insert(child,parent);
            return createIndex(row,column,(void*)child);
        }
    }
    if (row == 0 && column == 0 && !parent.isValid()) {
        // create root
        if (cs) {
            return createIndex(row, column, cs->list);
        }
        else {
            return createIndex(0,0,nullptr);
        }
    }
    return QModelIndex();
}

int MediasourceModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid() && parent.internalPointer()) {
        ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(parent.internalPointer());
        return ms->tree_item_get_children_count(it);
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
    switch(role) {
        case Qt::DisplayRole: {
            if (ms->scanner_state(source) != DDB_MEDIASOURCE_STATE_IDLE && !index.parent().isValid()) {
                const char *state_str[] = {nullptr, "loading", "scanning", "indexing", "saving"};
                ret = QString("Medialib is %1...") .arg(state_str[ms->scanner_state(source)]);
                break;
            }
            if (index.isValid() && index.internalPointer()) {
                ddb_medialib_item_t *it = static_cast<ddb_medialib_item_t *>(index.internalPointer());
                ret = ms->tree_item_get_text(it);
            }
            break;
        }
    }
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
    const ddb_medialib_item_t *child = ms->tree_item_get_children(l);//l->children;
    while (child) {
        playItemList pil_child = tracks((ddb_medialib_item_t *)child);
        foreach(DB_playItem_t *it, pil_child) {
            if (!pil.contains(it)) {
                pil.append(it);
                DBAPI->pl_item_ref(it);
            }
        }
        child = ms->tree_item_get_next(child);//child->next;
    }
    if (ms->tree_item_get_track(l) && !pil.contains(ms->tree_item_get_track(l))) {
        DBAPI->pl_item_ref(ms->tree_item_get_track(l));
        pil.append(ms->tree_item_get_track(l));
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

QStringList MediasourceModel::getDirectories() {
    QStringList l;
    size_t count;
    if (mlp) {
        char **folders = mlp->get_folders(source, &count);
        if (folders) {
            for (size_t i = 0; i < count; i++) {
                l.append(folders[i]);
            }
        }
        mlp->free_folders(source, folders, count);
    }
    return l;
}

void MediasourceModel::setDirectories(QStringList folders) {
    if (!mlp) {
        qDebug() << "MediasourceModel: setDirectories called but not supported on this model!";
        return;
    }

    QVector<const char*> vec;
    if (folders.length() > 0) {
        foreach(QString str, folders) {
            vec.append(strdup(str.toUtf8().constData()));
        }
        mlp->set_folders(source,vec.data(),folders.length());
        foreach(const char* str, vec) {
            free((char *) str);
        }
    }
    else {
        mlp->set_folders(source, nullptr, 0);
    }
    ms->refresh(source);
}

QModelIndex MediasourceModel::indexByPath(QStringList &l) {
    int i = 0;
    QModelIndex idx = QModelIndex();
    ddb_medialib_item_t *it = cs->list;
    int row = 0;
    while (it && i < l.length()) {
        if (l[i] == QString(ms->tree_item_get_text(it))) {
            idx = index(row,0,idx);
            if (i+1 < l.length()) {
                it = (ddb_medialib_item_t *) ms->tree_item_get_children(it);
                row = 0;
                i++;
            }
            else {
                break;
            }
        }
        else {
            it = (ddb_medialib_item_t *) ms->tree_item_get_next(it);
            row++;
        }
    }
    if (it && l.last() == QString(ms->tree_item_get_text(it))) {
        return idx;
    }
    return QModelIndex();
}
