#include "MediasourceModel.h"
#include <QThread>

#include "medialib.h"
#include "ScriptableSimple.h"

MediasourceModel::MediasourceModel(QObject *parent, DB_mediasource_t *Ms, QString source_name) : QAbstractItemModel(parent) {
    ms = Ms;
    source = ms->create_source(source_name.toUtf8().constData());

    listener = ms->add_listener(source,source_listener,this);

    if (!ms->is_source_enabled(source)) {
        ms->set_source_enabled(source, true);
    }

    ddb_scriptable_item_t *scripts = ms->get_queries_scriptable(source);
    QMap<QString, ddb_scriptable_item_t *> script_map = ScriptableSimple::getSelectors(scripts);
    m_script = script_map.values()[1];

    connect(this, &MediasourceModel::mediasourceNeedsToreload, this, &MediasourceModel::recreateTree);

    if (strcmp(ms->plugin.id, "medialib") == 0) {
        setProperty("isMedialib", true);
    }
}

MediasourceModel::~MediasourceModel() {
    ms->remove_listener(source,listener);
    if (root) {
        ms->free_item_tree(source, root);
    }

    ms->free_source(source);
}

ddb_mediasource_source_t* MediasourceModel::getSource() {
    return source;
}

void
MediasourceModel::setScriptable(ddb_scriptable_item_t *s) {
    m_script = s;
    recreateTree();
}

void
MediasourceModel::recreateTree() {
    beginResetModel();
    if (root) {
        ms->free_item_tree(source, root);
    }
    if (m_script) {
        root = ms->create_item_tree(source, m_script, filter.isEmpty()? nullptr : filter.toUtf8().constData());
    }
    endResetModel();
}

void MediasourceModel::source_listener(ddb_mediasource_event_type_t event, void *user_data) {
    MediasourceModel *m = static_cast<MediasourceModel*>(user_data);
    if (event == DDB_MEDIASOURCE_EVENT_CONTENT_DID_CHANGE ||
        event == DDB_MEDIASOURCE_EVENT_OUT_OF_SYNC) {
        emit m->mediasourceNeedsToreload();
    }
    if (m->property("isMedialib").toBool()) {
        if ((ddb_medialib_mediasource_event_type_t) event == DDB_MEDIALIB_MEDIASOURCE_EVENT_FOLDERS_DID_CHANGE) {
            emit m->mediasourceNeedsToreload();
        }
    }
}

QModelIndex MediasourceModel::index(int row, int column, const QModelIndex &parent) const {
    const ddb_medialib_item_t* it = toMedialibItem(parent);
    if (!parent.isValid() ) {
        it = root;
    }
    // go into specific item and create model index
    if (it) {
        const ddb_medialib_item_t *child = ms->tree_item_get_children(it);
        int i = row;
        while (i) {
            if (child)
                child = ms->tree_item_get_next(child);
            i--;
        }
        return createIndex(row,column,(void*)child);
    }
    //}
//    if (!parent.isValid()) {
//        return createIndex(row, column, root);
//    }
    return QModelIndex();
}

int MediasourceModel::rowCount(const QModelIndex &parent) const {
    const ddb_medialib_item_t *it = toMedialibItem(parent);
    if (it) {
        qDebug() << "count sub:" <<  ms->tree_item_get_children_count(it);
        return ms->tree_item_get_children_count(it);
    }
    else {
        qDebug() << "count:" <<  ms->tree_item_get_children_count(root);
        return ms->tree_item_get_children_count(root);
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
    ddb_medialib_item_t *it = toMedialibItem(index);
    switch(role) {
        case Qt::DisplayRole: {
            if (ms->scanner_state(source) != DDB_MEDIASOURCE_STATE_IDLE && !index.parent().isValid() ||
                root == nullptr || m_script == nullptr) {
                const char *state_str[] = {"idle", "loading", "scanning", "indexing", "saving"};
                ret = QString("Medialib is %1...") .arg(state_str[ms->scanner_state(source)]);
                break;
            }
            if (it) {
                ret = ms->tree_item_get_text(it);
            }
            break;
        }
        case MSRole::IsSelected: {
            if (it) {
                ret = (bool) ms->is_tree_item_selected(source, it);
            }
            break;
        }
        case MSRole::IsPartiallySelected: {
            if (it) {
                //ret = (bool) ms->is_tree_item_selected(source, it);
                bool some_selected = false;
                bool some_unselected = false;
                for (int i = 0; i < rowCount(index); i++) {
                    bool child_sel = data(this->index(i,0,index), MSRole::IsSelected).toBool();
                    bool child_par = data(this->index(i,0,index), MSRole::IsPartiallySelected).toBool();
                    if (child_sel) {
                        some_selected = true;
                    }
                    else {
                        some_unselected = true;
                    }
                }
                ret = some_selected && some_unselected;
            }
            break;
        }
        case MSRole::IsExpanded: {
            if (it) {
                ret = (bool) ms->is_tree_item_expanded(source, it);
            }
            break;
        }
    }
    return ret;
}

bool MediasourceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    ddb_medialib_item_t *it = toMedialibItem(index);
    switch(role) {
        case MSRole::IsSelected: {
            ms->set_tree_item_selected(source, it, value.toBool());
            emit dataChanged(index,index, {MSRole::IsSelected});
            return true;
            break;
        }
    }
    return false;
}

QModelIndex MediasourceModel::parent(const QModelIndex &index) const {
    ddb_medialib_item_t *it = toMedialibItem(index);
    if (it) {
            // fuck const
            return createIndex(itemRow(index),0, ms->get_tree_item_parent(it));
    }
    return QModelIndex();
}

QHash<int, QByteArray> MediasourceModel::roleNames() const {
    QHash<int, QByteArray> l;
    l.insert(Qt::DisplayRole, "display");
    l.insert(MSRole::IsSelected, "IsSelected");
    l.insert(MSRole::IsPartiallySelected, "IsPartiallySelected");
    l.insert(MSRole::IsExpanded, "IsExpanded");
    return l;
}
