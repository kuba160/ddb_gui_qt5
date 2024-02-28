#include "WidgetLibraryModel.h"

WidgetNode::WidgetNode(WidgetNode *parent) : QObject(parent) {
    this->parent = parent;
}

WidgetNode::~WidgetNode() {
}


WidgetNode *WidgetNode::createRootNode() {
    WidgetNode *n = new WidgetNode(nullptr);
    n->type = NodeRoot;
    return n;
}

void WidgetNode::createWidgetNode(QObject *widget) {
    Q_ASSERT(this->type == NodeRoot);

    WidgetNode *n = new WidgetNode(this);
    n->type = NodeWidget;
    n->friendly_info = widget->property("friendlyName").toString();
    n->internal_info = widget->property("internalName").toString();
    n->createStyleNode(widget);
    // n->relevant_widgets.append(widget);
     this->relevant_widgets.append(widget);
     this->children.append(n);
}

void WidgetNode::createStyleNode(QObject *widget) {
    Q_ASSERT(this->type == NodeWidget);

    WidgetNode *n = new WidgetNode(this);
    n->type = NodeStyle;
    n->friendly_info = widget->property("friendlyName").toString();
    n->internal_info = widget->property("internalName").toString();
    n->relevant_widgets.append(widget);
    this->relevant_widgets.append(widget);
    this->children.append(n);

    sortChildren();
}

void WidgetNode::insertToRoot(QObject *widget) {
    // we are root, find existing widget if available
    QString internalName = widget->property("internalName").toString();

    // find existing
    WidgetNode *n = findChildren(internalName);
    if (n == nullptr) {
        createWidgetNode(widget);
        sortChildren();
    }
    else {
        n->createStyleNode(widget);
    }

    //n->createStyleNode(widget);
    qDebug() << "WidgetNode: added" << widget->property("internalName").toString() << widget->property("widgetStyle").toString();
    qDebug() << "WidgetNode: root updated, main widgets:" << children.count() << ", with separate styles:" << relevant_widgets.count();
}

WidgetNode* WidgetNode::findChildren(QString internal_name) {
    for(WidgetNode *child : std::as_const(children)) {
        if (child->internal_info == internal_name) {
            return child;
        }
    }
    return nullptr;
}


void WidgetNode::sortChildren() {
    if (children.count() > 1) {
        std::sort(children.begin(), children.end(), WidgetNode::lessThan);
    }
}

WidgetLibraryModel::WidgetLibraryModel(QObject *parent, QList<QObject*> widgets)
    : QAbstractItemModel{parent} {

    root = WidgetNode::createRootNode();
    insert(widgets);
}

WidgetLibraryModel::~WidgetLibraryModel() {
    delete root;
}

void WidgetLibraryModel::insert(QList<QObject *> widgets) {
    for (QObject *obj : widgets) {
        insert(obj);
    }
}

void WidgetLibraryModel::insert(QObject *widget) {
    root->insertToRoot(widget);
}

QHash<int, QByteArray> WidgetLibraryModel::roleNames() const {
    return QHash<int, QByteArray>{
        {WidgetFriendlyName, "WidgetFriendlyName"},
        {WidgetInternalName, "WidgetInternalName"},
        {WidgetStyles, "WidgetStyles"},
        {WidgetTypes, "WidgetTypes"},
        {WidgetFormats, "WidgetFormats"},
        {WidgetQmlUrls, "WidgetQmlUrls"}
    };
}

int WidgetLibraryModel::rowCount (const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return root->children.count();
    }
    else {
        WidgetNode *n = static_cast<WidgetNode*>(parent.internalPointer());
        if (n->children.count() == 1) {
            return 0;
        }
        return n->children.count();
    }
    // for styles
    QStringList keys = map->uniqueKeys();
    QString key = keys[parent.row()];
    int total_styles = map->count(key);
    if (total_styles > 1) {
        return total_styles;
    }
    return 0;
}

int WidgetLibraryModel::columnCount (const QModelIndex &parent) const {
    // todo prob. needs fix
    if (!parent.isValid()) {
        return 1;
    }
    return 0;
}

QModelIndex WidgetLibraryModel::index(int row, int column, const QModelIndex &parent) const {
    if (column == 0) {
        WidgetNode *n = root;
        if (parent.isValid()) {
            n = static_cast<WidgetNode*>(parent.internalPointer());
        }
        if (row >= 0 && row < n->children.count()) {
            return createIndex(row, column, n->children[row]);
        }
    }
    return {};
}

QModelIndex WidgetLibraryModel::parent(const QModelIndex &index) const {
    if (index.isValid()) {
        WidgetNode *n = static_cast<WidgetNode*>(index.internalPointer());
        WidgetNode *n_parent = n->parent;
        if (n_parent != nullptr) {
            for (int i = 0 ; i < n_parent->children.count(); i++) {
                if (n == n_parent->children[i]) {
                    return createIndex(i, 0, n_parent);
                }
            }
        }
    }
    return {};
}


QVariant WidgetLibraryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    WidgetNode *n = static_cast<WidgetNode*>(index.internalPointer());
    Q_ASSERT(n != nullptr);
    switch (role) {
        case Qt::DisplayRole:
        case WidgetFriendlyName:
            if (n->relevant_widgets.count() == 1) {
                return QString("%1 (%2)").arg(n->friendly_info, n->relevant_widgets[0]->property("widgetStyle").toString());
            }
            return n->friendly_info;
        case WidgetInternalName:
            return n->internal_info;
        case WidgetStyles:
        case WidgetFormats:
        case WidgetTypes:
        case WidgetQmlUrls:
            if (n->type == WidgetNode::NodeWidget || n->type == WidgetNode::NodeStyle) {
                const char *prop = "";
                switch (role) {
                case WidgetStyles:
                    prop = "widgetStyle";
                    break;
                case WidgetFormats:
                    prop = "widgetFormat";
                    break;
                case WidgetTypes:
                    prop = "widgetType";
                    break;
                }

                QStringList names;
                for (QObject *value : n->relevant_widgets) {
                    if (role == WidgetQmlUrls) {
                        QString url = value->property("widgetUrl").toString();
                        if (!url.isEmpty())
                            names.append(url);
                    }
                    else {
                        names.append(value->property(prop).toString());
                    }
                }
                return names;

            }
            break;
    }
    return QVariant();
}

QList<QObject*> WidgetLibraryModel::getWidgets(QString internalName, QString style, QString type) {
    QList<QObject*> list;
    WidgetNode *node = root->findChildren(internalName);
    if (node) {
        for (QObject *obj : node->relevant_widgets) {
            if (!style.isNull() && obj->property("widgetStyle").toString() != style) {
                continue;
            }
            if (!type.isNull() && obj->property("widgetType").toString() != type) {
                continue;
            }
            list.append(obj);
        }
    }
    return list;
}

