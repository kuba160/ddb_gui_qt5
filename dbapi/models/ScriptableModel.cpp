#include "ScriptableModel.h"

ScriptableModel::ScriptableModel(QObject *parent, scriptableItem_t *root)
    : QAbstractItemModel{parent} {
    m_root = root;
}

int ScriptableModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return scriptableItemNumChildren(idxToItem(parent));
    }
    return 1;
}
int ScriptableModel::columnCount(const QModelIndex &parent) const {
    return m_root ? 1 : 0;
}

QModelIndex ScriptableModel::index(int row, int column, const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return createIndex(row, column, m_root);
    }
    //
    if (row < rowCount(parent) && column == 0) {
        return createIndex(row,column, scriptableItemChildAtIndex(idxToItem(parent), row));
    }
    return {};
}

QModelIndex ScriptableModel::parent(const QModelIndex &index) const {
    if (index.isValid()) {
        scriptableItem_t *p = scriptableItemParent(idxToItem(index));
        if (p) {
            scriptableItem_t *pp = scriptableItemParent(p);
            int row = pp ? scriptableItemIndexOfChild(pp,p) : 0;
            return createIndex(row, 0, p);
        }
    }
    return {};
}

QVariant ScriptableModel::data(const QModelIndex &index, int role) const {
    if (index.isValid()) {
        if (roleToKeyMap.contains(role)) {
            return scriptableItemPropertyValueForKey(idxToItem(index), roleToKeyMap.value(role));
        }
    }
    return {};
}

bool ScriptableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid()) {
        if (role == Qt::EditRole) {
            role = Qt::DisplayRole; // LOL HACK
        }
        if (roleToKeyMap.contains(role)) {
            scriptableItemSetPropertyValueForKey(idxToItem(index),
                                                 value.toString().toUtf8().constData(),
                                                 roleToKeyMap.value(role));
            return true;
        }
    }
    return false;
}

Qt::ItemFlags ScriptableModel::flags(const QModelIndex &index) const {
    uint64_t scriptable_flags = scriptableItemFlags(idxToItem(index));
    Qt::ItemFlags f = Qt::NoItemFlags;

    if (scriptable_flags & SCRIPTABLE_FLAG_IS_READONLY) {
        f = f | Qt::ItemIsEditable;
    }
    if (scriptable_flags & SCRIPTABLE_FLAG_IS_REORDABLE) {
        f = f | Qt::ItemIsDragEnabled;
    }
    return f;
}

//QStringList ScriptableModel::getPresetNames() {
//    QStringList l;
//    for (int i = 0; i < scriptableItemNumChildren(m_root); i++) {
//        scriptableItem_t *it = scriptableItemChildAtIndex(m_root,i);
//        l.append(scriptableItemPropertyValueForKey(it, "name"));
//    }
//    return l;
//}
