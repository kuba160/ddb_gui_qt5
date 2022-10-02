#ifndef DROLEMAPEPROXYMODEL_H
#define DROLEMAPEPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QHash>

class DRoleMapProxyModel : public QAbstractProxyModel {
    Q_OBJECT
    QList<QHash<int, int>> roles;
    QHash<int, QString> titles;
public:
    explicit DRoleMapProxyModel(QObject *parent = nullptr);

    QModelIndex parent(const QModelIndex &) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;


    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
};

#endif // DROLEMAPEPROXYMODEL_H
