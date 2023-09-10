#include "ActionsModel.h"
#include <QDebug>

#include <QRegularExpression>

//#define DBAPI (this->api)

QStringList extractActionsOrdering(QString prototype_spec, QString property);

ActionsModel::ActionsModel(Actions *Actions, uint16_t action_filter, QString prototype_spec)
    : QAbstractItemModel{Actions} {
    actions = Actions;
    location_filter = action_filter;
    prototype = prototype_spec;

    if (!prototype_spec.isNull()) {
        order = extractActionsOrdering(prototype_spec, {});
        order_first = extractActionsOrdering(prototype_spec, "first");
        order_last = extractActionsOrdering(prototype_spec, "last");
        order_extract = extractActionsOrdering(prototype_spec, "extract");
    }

    rebuildActionTree();

    connect(actions, &Actions::actionsAdded, this, [=]() {rebuildActionTree();});
}

ActionsModel::~ActionsModel() {
    delete tree;
}

QStringList extractActionsOrdering(QString prototype_spec, QString property = QString()) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(prototype_spec.toUtf8(), &error);
    if (doc.isNull()) {
        qDebug() << "Failed to parse prototype_spec:" << error.errorString();
        return {};
    }
    QStringList out_list;
    QJsonArray actions = doc.array();
    for (QJsonValue v : actions) {
        QString action = QString();
        if (v.isObject()) {
            QJsonObject obj = v.toObject();
            if (obj.contains("action")) {
                if (!property.isNull()) {
                    if (obj.contains("properties")) {
                        QJsonObject props = obj.value("properties").toObject();
                        if (props.contains(property)) {
                            if (props.value(property).toBool()) {
                                // ifififififif
                                action = obj.value("action").toString();
                            }
                        }

                    }
                }
                else {
                    action = obj.value("action").toString();
                }
            }
        }
        if (!action.isNull()) {
            out_list.append(action);
        }
    }
    return out_list;
}

QHash<QString, QHash<QString,QVariant>> extractActionsPrototypes(QString prototype_spec, QStringList action_list) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(prototype_spec.toUtf8(), &error);
    if (doc.isNull()) {
        qDebug() << "Failed to parse prototype_spec:" << error.errorString();
        return {};
    }
    QHash<QString, QHash<QString, QVariant>> out_list;
    QJsonArray actions = doc.array();
    for (QJsonValue v : actions) {
        QString action = QString();
        if (v.isObject()) {
            QJsonObject obj = v.toObject();
            if (obj.contains("action")) {
                action = obj.value("action").toString();
                if (actions.contains(action)) {
                    if (obj.contains("properties")) {
                        QJsonObject props = obj.value("properties").toObject();
                        out_list.insert(action, props.toVariantHash());
                    }
                }
            }
        }
    }
    return out_list;
}

void ActionsModel::rebuildActionTree() {
    beginResetModel();
    delete tree;
    tree = new TreeNode<QString>();

    QStringList all_actions = actions->getActions();
    QStringList actions_added;
    if (!all_actions.size()) {
        return;
    }
    QList<QStringList> paths_first;
    QList<QStringList> paths_last;
    // first round of adding from prototype
    for (QString &action_id : order) {
        DBAction *action = actions->getAction(action_id);
        if (action && action->locations & location_filter) {
            QStringList order_path;
            if (order_extract.contains(action_id)) {
                tree->insertChild(action->title, action->action_id);
                order_path = {action->title};
            }
            else {
                order_path = action->path;
                tree->insertChildIter(action->path, action->action_id);
                //qDebug() << "Inserting:" << action->path << action->action_id;
            }

            if (order_first.contains(action->action_id)) {
                paths_first.append(order_path);
            }
            if (order_last.contains(action->action_id)) {
                paths_last.append(order_path);
            }
        }
        actions_added.append(action_id);
        all_actions.removeAll(action_id);
    }
    // second round of adding actions missing from prototype
    while (!all_actions.isEmpty()) {
        QString action_id = all_actions.takeFirst();
        DBAction *action = actions->getAction(action_id);
        if (action && action->locations & location_filter) {
            QStringList order_path;
            if (order_extract.contains(action_id)) {
                tree->insertChild(action->title, action->action_id);
                order_path = {action->title};
            }
            else {
                order_path = action->path;
                tree->insertChildIter(action->path, action->action_id);
                //qDebug() << "Inserting:" << action->path << action->action_id;
            }

            if (order_first.contains(action->action_id)) {
                paths_first.append(order_path);
            }
            if (order_last.contains(action->action_id)) {
                paths_last.append(order_path);
            }
        }
        actions_added.append(action_id);
        all_actions.removeAll(action_id);
    }

    // first/last sorting

    for (QStringList &path : paths_first) {
        tree->pushChildrenFirst(path);
    }
    for (QStringList &path : paths_last) {
        tree->pushChildrenLast(path);
    }

    prototype_overrides = extractActionsPrototypes(prototype, actions_added);

    endResetModel();
}

QJsonObject ActionsModel::toJson(QModelIndex idx, PlayItemIterator &pit) {
    QJsonObject out;
    bool isSubmenu = hasChildren(idx);
    if (isSubmenu) {
        out.insert("type", "submenu");
        out.insert("text", idx.data().toString());
        //qDebug() << "TEST" << idx.data().toString();
        QJsonArray children;
        int children_count = rowCount(idx);
        for (int i = 0; i < children_count; i++) {
            QModelIndex next = index(i,0,idx);
            QJsonObject child = toJson(next, pit);
            children.append(child);
        }
        out.insert("children", children);
    }
    else {
        out.insert("type", "action");
        out.insert("text", idx.data().toString());
        QString action_id = data(idx, Qt::UserRole).toString();
        DBAction *action = actions->getAction(action_id);
        if (action) {
            out.insert("id", action->action_id);
            QHash<QString,QVariant> props = action->properties_const;
            props.insert(action->contextualize(pit));
            // override
            if (prototype_overrides.contains(action->action_id)) {
                props.insert(prototype_overrides.value(action->action_id));
            }
            QJsonObject properties;
            for (auto key : props.keys()) {
                properties.insert(key, props.value(key).toJsonValue());
            }
            out.insert("properties", properties);
        }
    }
    return out;
}

int ActionsModel::rowCount(const QModelIndex &idx) const {
    const TreeNode<QString> *node = this->getNodeForIndex(idx);
    if (node)
        return node->getChildrenCount();
    return 0;
}

int ActionsModel::columnCount(const QModelIndex &idx) const {
    const TreeNode<QString> *node = this->getNodeForIndex(idx);
    if (node)
        return 1;
    return 0;
}

QModelIndex ActionsModel::index(int row, int column, const QModelIndex &parent) const {
    if (column > 0) {
        return {};
    }
    TreeNode<QString> *node = getNodeForIndex(parent);
    if (node) {
        if (row >= node->getChildrenCount()) {
            return {};
        }
        return createIndex(row, column, node->getChild(row));
    }
    return {};
}

QModelIndex ActionsModel::parent(const QModelIndex &index) const {
    TreeNode<QString> *node = getNodeForIndex(index);
    if (node) {
        TreeNode<QString> *parent = node->getParent();
        if (parent) {
            return createIndex(0, 0, parent);
        }
    }
    return {};
}

bool ActionsModel::hasChildren(const QModelIndex &parent) const {
    return rowCount(parent);
}

QVariant ActionsModel::data(const QModelIndex &index, int role) const {
    TreeNode<QString> *node = getNodeForIndex(index);
    if (node) {
        if (role == Qt::DisplayRole) {
            return node->title;
        }
        else if (role == Qt::CheckStateRole) {
            if (node->leafNode) {
                DBAction *action = actions->getAction(node->value);
                PlayItemIterator pit(false);
                QHash<QString, QVariant> props = action->contextualize(pit);
                if (props.contains("checked")) {
                    return props.value("checked").toBool();
                }
            }
        }
        else if (role == Qt::UserRole) {
            if (node->leafNode)
                return node->value;
        }
    }
    return QVariant();
}

template class TreeNode<QString>;
