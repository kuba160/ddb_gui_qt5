#include "PlayItemFilterModel.h"

#include "PlayItemModel.h"

PlayItemFilterModel::PlayItemFilterModel(QObject *parent)
    : QSortFilterProxyModel{parent} {

}

QString PlayItemFilterModel::getItemFilter() {
    return m_filter;
}
void PlayItemFilterModel::setItemFilter(QString filter) {
    m_filter = filter;
    emit itemFilterChanged();
    invalidateFilter();
}

bool PlayItemFilterModel::filterAcceptsRow(int sourceRow,
                                              const QModelIndex &sourceParent) const {
    if (!sourceModel() || m_filter.isEmpty()) {
        return false;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const QList<int> filter_roles ({PlayItemModel::ItemArtist,
                              PlayItemModel::ItemAlbum,
                              PlayItemModel::ItemTitle});

    for (int role : filter_roles) {
        QString to_compare = sourceModel()->data(index, role).toString();
        if (to_compare.contains(m_filter, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}
