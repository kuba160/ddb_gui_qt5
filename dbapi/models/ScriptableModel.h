#ifndef SCRIPTABLEMODEL_H
#define SCRIPTABLEMODEL_H

#include <QAbstractItemModel>
#include "../scriptable/scriptable.h"

class ScriptableModel : public QAbstractItemModel
{
public:
    explicit ScriptableModel(QObject *parent = nullptr, scriptableItem_t *root = nullptr);


    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    static scriptableItem_t* idxToItem(const QModelIndex &idx) {
        return static_cast<scriptableItem_t*>(idx.internalPointer());
    };


    void mapRole(int role, const char *key) {
        roleToKeyMap.insert(role, key);
    }

    void marRoleRemove(int role) {
        roleToKeyMap.remove(role);
    }

    //QStringList getPresetNames();

private:
    scriptableItem_t *m_root;
    QMap<int,const char *> roleToKeyMap;
};

#endif // SCRIPTABLEMODEL_H
