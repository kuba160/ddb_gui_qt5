#include "DRoleMapProxyModel.h"
#include <QAbstractItemModel>
#include <QDebug>

DRoleMapProxyModel::DRoleMapProxyModel(QObject *parent)
    : QAbstractProxyModel{parent}
{

}

QModelIndex DRoleMapProxyModel::parent(const QModelIndex &) const {
    return {};
}

QModelIndex DRoleMapProxyModel::index(int row, int column, const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return createIndex(row, column);
    }
    return {};
}

void DRoleMapProxyModel::setSourceModel(QAbstractItemModel *new_source_model) {
    if (sourceModel()) {
        disconnect(sourceModel(), nullptr, this, nullptr);
    }
    if (new_source_model) {
        connect(new_source_model, &QObject::destroyed, this, [this]{setSourceModel(nullptr);});

        connect(new_source_model, &QAbstractItemModel::modelAboutToBeReset, this, [this] {beginResetModel();});
        connect(new_source_model, &QAbstractItemModel::modelReset, this, [this] {endResetModel();});
        connect(new_source_model, &QAbstractItemModel::rowsAboutToBeInserted, this,
                [this](const QModelIndex &parent, int start, int end) {beginInsertRows(mapFromSource(parent),start, end);});
        connect(new_source_model, &QAbstractItemModel::rowsInserted, this, [this] {endInsertRows();});

        connect(new_source_model, &QAbstractItemModel::rowsAboutToBeMoved, this,
                [this](const QModelIndex &parent, int start, int end, const QModelIndex &destinationParent, int destination_row) {
            beginMoveRows(mapFromSource(parent), start, end, mapFromSource(destinationParent), destination_row);
        });
        connect(new_source_model, &QAbstractItemModel::rowsMoved, this, [this] {endMoveRows();});

        connect(new_source_model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
                [this](const QModelIndex &parent, int first, int last) {beginRemoveRows(parent, first, last);});
        connect(new_source_model, &QAbstractItemModel::rowsRemoved, this, [this] {endRemoveRows();});

        connect(new_source_model, &QAbstractItemModel::dataChanged, this,
                [this] (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QList<int> &roles = QList<int>()) {
            if (roles.isEmpty()) {
                // we don't know what data changed, updating all columns
                emit dataChanged(createIndex(topLeft.row(),0), createIndex(bottomRight.row(), columnCount()));
            }
            else {
                // go through each column
                for (int col = 0; col < this->roles.count() ; col++) {
                    // role_map for given column
                    QHash<int,int> column_role_map = this->roles.at(col);
                    // go through each source roles
                    for (int source_role : roles) {
                        // find any key in role_map that maps to given source_role
                        for (int role_key : column_role_map) {
                            if (source_role == role_key) {
                                emit dataChanged(createIndex(topLeft.row(), col),
                                                 createIndex(bottomRight.row(), col),
                                                 {column_role_map.value(role_key), Qt::DisplayRole, Qt::DecorationRole});
                            }
                        }
                    }
                }
            }
        });
    }

    // layout?

    QAbstractProxyModel::setSourceModel(new_source_model);
}

QModelIndex DRoleMapProxyModel::mapFromSource(const QModelIndex &sourceIndex) const {
    if (sourceModel()) {
        return createIndex(sourceIndex.row(), sourceIndex.column());
    }
    return {};
}

QModelIndex DRoleMapProxyModel::mapToSource(const QModelIndex &proxyIndex) const {
    if (sourceModel()) {
        return sourceModel()->index(proxyIndex.row(),proxyIndex.column());
    }
    return {};
}

int DRoleMapProxyModel::rowCount(const QModelIndex &parent) const {
    if (sourceModel() && !parent.isValid()) {
        return sourceModel()->rowCount(parent);
    }
    return 0;
}

int DRoleMapProxyModel::columnCount(const QModelIndex &parent) const {
    if (sourceModel() && !parent.isValid()) {
        return roles.count();
    }
    return 0;
}

QVariant DRoleMapProxyModel::data(const QModelIndex &index, int role) const {
    if (sourceModel()) {
        if (index.column() < roles.size()) {
            QHash<int,int> role_map = roles.at(index.column());
            if (role_map.contains(role)) {
                // return mapped value
                return sourceModel()->data(mapToSource(index), role_map.value(role));
            }
            else {
                // return non-mapped value
                return sourceModel()->data(mapToSource(index), role);
            }
        }
    }
    return {};
}

QVariant DRoleMapProxyModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (sourceModel()) {
        // title role override
        if (role == Qt::DisplayRole && titles.contains(section)) {
            return titles.value(section);
        }
        // return headerData for given role map
        if (section < roles.size()) {
            QHash<int,int> role_map = roles.at(section);
            if (role_map.contains(role)) {
                return sourceModel()->headerData(0, orientation, role_map.value(role));
            }
        }
        // return unmodified sourceModel headerData
        return sourceModel()->headerData(section, orientation, role);
    }
    return {};
}

bool DRoleMapProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) {
    if (section >= 0 && section <= roles.count()) {
        if (role == Qt::EditRole) {
            if (!value.toString().isEmpty()) {
                titles.insert(section, value.toString());
                qDebug() << "TITLE inserted for section " << section;
            }
            else if (titles.contains(section)) {
                titles.remove(section);
            }
            else {
                return false;
            }
            emit headerDataChanged(Qt::Horizontal, section,section);
            return true;
        }
        // check if we insert
        if (section == roles.count()) {
            if (!value.isValid()) {
                return false;
            }
            beginInsertColumns({}, section, section);
            QHash<int,int> role_map;
            role_map.insert(role, value.toInt());
            roles.append(role_map);
            endInsertColumns();
            qDebug() << "NEW MAP inserted for section " << section;
        }
        else {
            if (!value.isValid()) {
                beginRemoveColumns({}, section, section);
                roles.removeAt(section);
                if (titles.contains(section)) {
                    titles.remove(section);
                }

                // shift titles
                QList<int> keys = titles.keys();
                for (int i : std::as_const(keys)) {
                    if (i > section) {
                        // TODO FIX THIS SHIT
                        QString title = titles.value(i);
                        titles.remove(i);
                        titles.insert(i - 1, title);
                        emit headerDataChanged(Qt::Horizontal, i,i);
                    }
                }
                endRemoveColumns();
                return true;
            }
            QHash<int,int> role_map = roles.at(section);
            role_map.insert(role, value.toInt());
            roles.replace(section, role_map);
            emit dataChanged(createIndex(0, section), createIndex(rowCount(), section));
            qDebug() << "MAP replaced for section " << section;
        }
        return true;
    }
    return false;
}
