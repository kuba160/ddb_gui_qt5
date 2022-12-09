#ifndef PLAYITEMFILTERMODEL_H
#define PLAYITEMFILTERMODEL_H

#include <QSortFilterProxyModel>

class PlayItemFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit PlayItemFilterModel(QObject *parent = nullptr);

    Q_PROPERTY(QString item_filter READ getItemFilter WRITE setItemFilter NOTIFY itemFilterChanged)
    QString getItemFilter();
    void setItemFilter(QString filter);


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

signals:
    void itemFilterChanged();

private:
    QString m_filter;
};

#endif // PLAYITEMFILTERMODEL_H
