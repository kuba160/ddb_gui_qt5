#include "MediasourceModel.h"
#include <QThread>
#include <QTimer>
#include "medialib.h"
#include "ScriptableModel.h"

MediasourceModel::MediasourceModel(QObject *parent, DB_mediasource_t *Ms, QString source_name) : QAbstractItemModel(parent) {
    ms = Ms;
    source = ms->create_source(source_name.toUtf8().constData());

    listener = ms->add_listener(source,source_listener,this);

    if (!ms->is_source_enabled(source)) {
        ms->set_source_enabled(source, true);
    }

    m_script = ms->get_queries_scriptable(source);

    m_script_model = new ScriptableModel(this, m_script);
    m_script_model->mapRole(Qt::DisplayRole, "name");


    connect(this, &MediasourceModel::mediasourceNeedsToreload, this, &MediasourceModel::recreateTree);
    connect(this, &MediasourceModel::presetIdxChanged, this, &MediasourceModel::recreateTree);
    connect(this, &MediasourceModel::filterChanged, this, &MediasourceModel::recreateTree);

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
    scriptableItem_t *preset = nullptr;
    if (m_script && m_preset_idx < scriptableItemNumChildren(m_script)) {
        preset = scriptableItemChildAtIndex(m_script, m_preset_idx);
    }
    if (m_script) {
        root = ms->create_item_tree(source, preset, m_filter.isEmpty()? nullptr : m_filter.toUtf8().constData());
    }
    // force root tree to be expanded
    if (root) {
        ms->set_tree_item_expanded(source, root, true);
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
    if (!it && row == 0 && column == 0) {
        if (root)
            return createIndex(0,0, (void*)root);
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
        //qDebug() << "count sub:" <<  ms->tree_item_get_children_count(it);
        return ms->tree_item_get_children_count(it);
    }
    else {
        return 1;
    }
    return 0;
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
    if (it) {
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
            case MSRole::IsPartiallySelected:
            case MSRole::IsSelected: {
                if (it) {
                    ret = (bool) ms->is_tree_item_selected(source, it);
                }
                break;
            }
    //        case MSRole::IsPartiallySelected: {
    //            if (it) {
    //                //ret = (bool) ms->is_tree_item_selected(source, it);
    //                bool some_selected = false;
    //                bool some_unselected = false;
    //                for (int i = 0; i < rowCount(index); i++) {
    //                    bool child_sel = data(this->index(i,0,index), MSRole::IsSelected).toBool();
    //                    bool child_par = data(this->index(i,0,index), MSRole::IsPartiallySelected).toBool();
    //                    if (child_sel) {
    //                        some_selected = true;
    //                    }
    //                    else {
    //                        some_unselected = true;
    //                    }
    //                }
    //                ret = some_selected && some_unselected;
    //            }
    //            break;
    //        }
            case MSRole::IsExpanded: {
                if (it) {
                    ret = (bool) ms->is_tree_item_expanded(source, it);
                }
                break;
            }
            case MSRole::HasChildren: {
                if (it) {
                    ret = (bool) ms->tree_item_get_children_count(it);
                }
                break;
            }
        }
    }
    return ret;
}

bool MediasourceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    ddb_medialib_item_t *it = toMedialibItem(index);
    if (it) {
        switch(role) {
            case MSRole::IsSelected: {
                ms->set_tree_item_selected(source, it, value.toBool());
                emit dataChanged(index,index, {MSRole::IsSelected});
                return true;
                break;
            }
            case MSRole::IsExpanded: {
                ms->set_tree_item_expanded(source, it, value.toBool());
                emit dataChanged(index,index, {MSRole::IsExpanded});
                return true;
                break;
            }
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
    l.insert(MSRole::HasChildren, "HasChildren");
    return l;
}

QStringList MediasourceModel::getPresets() {
    QModelIndex root = m_script_model->index(0,0,{});
    QStringList l;
    for (int i = 0; i < m_script_model->rowCount(root); i++) {
            QModelIndex curr = m_script_model->index(i,0,root);
            l.append(m_script_model->data(curr,Qt::DisplayRole).toString());
    }
    return l;
}

void MediasourceModel::setPresetIdx(int idx) {
    if (idx != m_preset_idx) {
        m_preset_idx = idx;
        emit presetIdxChanged();
    }
}
