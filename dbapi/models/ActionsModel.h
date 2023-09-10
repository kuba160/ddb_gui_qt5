#ifndef ACTIONSMODEL_H
#define ACTIONSMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <deadbeef/deadbeef.h>
#include "../Actions.h"

#include "TreeNode.h"
#include "../ActionOwner.h"

class ActionsModel : public QAbstractItemModel {
    Q_OBJECT
    TreeNode<QString> *tree = nullptr;
    Actions *actions = nullptr;
    uint16_t location_filter;

    // special handling of action order/extraction
    QStringList order;
    QStringList order_first;
    QStringList order_last;
    QStringList order_extract;
    QString prototype;
    QHash<QString, QHash<QString,QVariant>> prototype_overrides;

public:
    explicit ActionsModel(Actions *Actions, uint16_t action_filter = 0, QString prototype_spec = {});
    ~ActionsModel();


    enum ActionsModelRoles {
        // data
        ACTION_ID = Qt::UserRole,

    };

    void rebuildActionTree();

    //
//    void registerActionOwner(ActionOwner *);
//    void unregisterActionOwner(ActionOwner *);

    QJsonObject toJson(QModelIndex idx, PlayItemIterator &pit);

    //
    virtual int rowCount(const QModelIndex &parent = {}) const override;
    virtual int columnCount(const QModelIndex &parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //virtual bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::EditRole) override;

protected:

    inline TreeNode<QString> *getNodeForIndex(const QModelIndex &idx) const {
        return idx.isValid() ?
            static_cast<TreeNode<QString> *>(idx.internalPointer()) :
            tree;
    };
};


#endif // ACTIONSMODEL_H
