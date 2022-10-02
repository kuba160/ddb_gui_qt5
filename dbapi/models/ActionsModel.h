#ifndef ACTIONSMODEL_H
#define ACTIONSMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include <deadbeef/deadbeef.h>
#include "../Actions.h"


template <class T>
class TreeNode {
public:
    // create leaf node
    TreeNode(TreeNode *parent, QString title, T value);
    // create parent node
    TreeNode(TreeNode *parent = nullptr, QString title = {});

    // group text
    QString title;
    //
    bool leafNode;
    // value (default constructed for group node)
    T value;

    // parent node (for optimalization)
    TreeNode *parent;

    // insert leaf node (where title is separated by '/')
    void insertChild(QString title, T value);

    bool operator== (const TreeNode& cmp)  {
        return title == cmp.title;
    }

    // children operations
    int getChildrenCount() const { return children.count(); }
    TreeNode * getChild(int idx) const { return children.at(idx);}
    TreeNode * getParent() const { return parent;}
    int getNodeIndex() const { return parent->getChildIdx(this);}

    int getChildIdx(TreeNode *child) const {return child->children.indexOf(child);}

protected:
    void insertChildIter(QStringList title, T value);
    QList<TreeNode *> children;

};

class ActionsModel : public QAbstractItemModel {
    Q_OBJECT
    DB_functions_t *api;
    TreeNode<const DBAction *> *tree;
    QList<QVariant (*)(QObject*)> converters;

public:
    explicit ActionsModel(QObject *parent, DB_functions_t *Api, uint16_t action_filter = 0);


    enum ActionsModelRoles {
        // data
        ACTION_NODE = Qt::UserRole,
        // editData
        ACTION_EXECUTE, // no data
        ACTION_EXECUTE_MAIN = ACTION_EXECUTE, // no data
        ACTION_EXECUTE_SELECTION, // playitemlist/mime ???.
        ACTION_EXECUTE_PLAYLIST, // ddb_playlist_t*
        ACTION_EXECUTE_NOWPLAYING, // no data
    };
    Q_ENUM(ActionsModelRoles)
    static inline DBAction::ActionExecuteMode convertActionRoleToMode(int role) {
        switch (role) {
            //case ACTION_EXECUTE:
            case ACTION_EXECUTE_MAIN:
                return DBAction::ACTION_MAIN;
            case ACTION_EXECUTE_SELECTION:
                return DBAction::ACTION_SELECTION;
            case ACTION_EXECUTE_PLAYLIST:
                return DBAction::ACTION_PLAYLIST;
            case ACTION_EXECUTE_NOWPLAYING:
                return DBAction::ACTION_NOWPLAYING;
        }
        return DBAction::ACTION_MAIN;
    }

    void insertActions(const QList<DBAction*> action_list);

    //
    virtual int rowCount(const QModelIndex &parent = {}) const override;
    virtual int columnCount(const QModelIndex &parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::EditRole) override;

protected:

    inline TreeNode<const DBAction *> *getNodeForIndex(const QModelIndex &idx) const {
        return idx.isValid() ?
            static_cast<TreeNode<const DBAction *> *>(idx.internalPointer()) :
            tree;
    };
};


#endif // ACTIONSMODEL_H
