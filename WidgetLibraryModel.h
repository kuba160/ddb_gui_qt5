#ifndef WIDGETLIBRARYMODEL_H
#define WIDGETLIBRARYMODEL_H

#include <QAbstractListModel>
#include <QMultiMap>

class WidgetNode : public QObject {
    Q_OBJECT
public:
    WidgetNode(WidgetNode *parent = nullptr);
    ~WidgetNode();
    enum NodeType {
        NodeRoot,
        NodeWidget,
        NodeStyle
    };

    NodeType type;
    QString friendly_info;
    QString internal_info;
    QList<QObject *> relevant_widgets;
    QList<WidgetNode *> children;
    WidgetNode * parent;

    static WidgetNode *createRootNode();
    void createWidgetNode(QObject *widget);
    void createStyleNode(QObject *widget);
    void insertToRoot(QObject *widget);
    void sortChildren();


    WidgetNode* findChildren(QString internal_name);

    static bool lessThan(const WidgetNode *v1, const WidgetNode *v2) {
        return v1->friendly_info < v2->friendly_info;
    }


};

class WidgetLibraryModel : public QAbstractItemModel {
    Q_OBJECT
    QMultiMap<QString, QObject*> *map;

    WidgetNode *root;

public:
    WidgetLibraryModel(QObject *parent, QList<QObject*> widgets = QList<QObject*>());
    ~WidgetLibraryModel();

    enum WidgetLibraryNames {
        WidgetFriendlyName = Qt::UserRole,
        WidgetInternalName,
        WidgetStyles,
        WidgetTypes,
        WidgetFormats,
        WidgetQmlUrls
    };
    Q_ENUMS(WidgetLibraryNames)

    void insert(QObject *widget);
    void insert(QList<QObject*> widgets);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount (const QModelIndex &parent = QModelIndex()) const override;
    int columnCount (const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;


    QList<QObject*> getWidgets(QString internalName, QString style = QString(), QString type = QString());
};

#endif // WIDGETLIBRARYMODEL_H
