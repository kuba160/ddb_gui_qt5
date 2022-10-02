#include "ActionsModel.h"
#include <QDebug>

#include <QRegularExpression>

#define DBAPI (this->api)

ActionsModel::ActionsModel(QObject *parent, DB_functions_t *Api, uint16_t action_filter)
    : QAbstractItemModel{parent} {
    api = Api;
    tree = new TreeNode<const DBAction *>();
}

ActionsModel::~ActionsModel() {
    delete tree;
}

void ActionsModel::insertActions(const QList<DBAction*> action_list) {
    beginResetModel();
    for (const DBAction* action : action_list) {
        tree->insertChild(action->path, action);
    }
    endResetModel();
}

int ActionsModel::rowCount(const QModelIndex &idx) const {
    const TreeNode<const DBAction *> *node = this->getNodeForIndex(idx);
    if (node)
        return node->getChildrenCount();
    return 0;
}

int ActionsModel::columnCount(const QModelIndex &idx) const {
    const TreeNode<const DBAction *> *node = this->getNodeForIndex(idx);
    if (node)
        return 1;
    return 0;
}

QModelIndex ActionsModel::index(int row, int column, const QModelIndex &parent) const {
    if (column > 0) {
        return {};
    }
    TreeNode<const DBAction *> *node = getNodeForIndex(parent);
    if (node) {
        if (row >= node->getChildrenCount()) {
            return {};
        }
        return createIndex(row, column, node->getChild(row));
    }
    return {};
}

QModelIndex ActionsModel::parent(const QModelIndex &index) const {
    TreeNode<const DBAction *> *node = getNodeForIndex(index);
    if (node) {
        TreeNode<const DBAction *> *parent = node->getParent();
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
    TreeNode<const DBAction *> *node = getNodeForIndex(index);
    if (node) {
        if (role == Qt::DisplayRole) {
            if (node->leafNode)
                return node->value->title;
            else
                return node->title;
        }
        else if (role == ACTION_NODE) {
            return QVariant::fromValue((DBAction *) node->value);
        }
        if (role == Qt::UserRole) {
            return QVariant::fromValue((QObject *)node->value);
        }
    }
    return QVariant();
}

bool ActionsModel::setData(const QModelIndex &index, const QVariant &data, int role) {
    TreeNode<const DBAction *> *node = getNodeForIndex(index);
    if (node && node->leafNode) {
        const DBAction * action = node->value;
        action->actionExecute(convertActionRoleToMode(role));
        return true;
    }
    return false;
}

template <class T>
TreeNode<T>::TreeNode(TreeNode *parent, QString title, T value) {
    this->title = title;
    this->parent = parent;
    if (value != T{}) {
        this->value = value;
        this->leafNode = true;
    }
    else {
        this->leafNode = false;
    }
}

template <class T>
TreeNode<T>::TreeNode(TreeNode *parent, QString title) {
    this->title = title;
    this->value= T{};
    this->parent = parent;
    this->leafNode = false;
}

template <class T>
TreeNode<T>::~TreeNode() {
    for (TreeNode<T>* child : children) {
        delete child;
    }
}


template <class T>
void TreeNode<T>::insertChild(QString title, T value) {
    static QRegularExpression re("\\/(?<!\\\\\\/)"); // regex go brrrrr
    insertChildIter(QString(title).split(re), value);
}

template <class T>
void TreeNode<T>::insertChildIter(QStringList list, T value) {
    if (list.length() == 1) {
        children.append(new TreeNode(this, list[0], value));
    }
    else {
        for (TreeNode *i : children) {
            if (i->title == list[0]) {
                list.removeFirst();
                i->insertChildIter(list, value);
                return;
            }
        }
        // no child node found, create a new one
        TreeNode<T> *child = new TreeNode<T>(this, list[0], T{});
        children.append(child);
        list.removeFirst();
        child->insertChildIter(list, value);
    }
}
