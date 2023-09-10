#include "PlayItemTableProxyModel.h"

#include "PlayItemModel.h"

#include <QDataStream>
#include <QIODevice>

PlayItemTableProxyModel::PlayItemTableProxyModel(QObject *parent)
    : DRoleMapProxyModel{parent}
{

}

QAbstractItemModel * PlayItemTableProxyModel::getPlayItemModel() {
    PlayItemModel *m = qobject_cast<PlayItemModel *>(this->sourceModel());
    QAbstractProxyModel *m2 = qobject_cast<QAbstractProxyModel *>(this->sourceModel());
    while (!m && m2) {
        m = qobject_cast<PlayItemModel *>(m2->sourceModel());
        m2 = qobject_cast<QAbstractProxyModel *>(m2->sourceModel());
    }
    return m;
}

void PlayItemTableProxyModel::setSourceModel(QAbstractItemModel *sourceModel) {
    /*
    if (custom_roles_queued.count()) {
        PlayItemModel *m = qobject_cast<PlayItemModel *>(sourceModel);
        QAbstractProxyModel *m2 = qobject_cast<QAbstractProxyModel *>(sourceModel);
        while (!m && m2) {
            qDebug() << "111M" << m << ",M2" << m2;
            m = qobject_cast<PlayItemModel *>(m2->sourceModel());
            m2 = qobject_cast<QAbstractProxyModel *>(m2->sourceModel());
        }
        qDebug() << "222M" << m << ",M2" << m2;

        return;
        //if (!m)
        for (HeaderData_t hd : qAsConst(custom_roles_queued)) {

        }
    }*/

    connect (sourceModel, &QAbstractItemModel::modelReset, this, [this]() {
        // try to iterate over custom roles, register custom formats and set them to role map
        if (custom_roles_queued.count()) {
            PlayItemModel *m = qobject_cast<PlayItemModel*>(getPlayItemModel());
            if (m) {
                QList<int> l = custom_roles_queued.keys();
                for (int i : qAsConst(l)) {
                    HeaderData_t hd = custom_roles_queued.value(i);
                    setRoleMap(i, hd);
                }
                custom_roles_queued.clear();
            }
        }
    });
    connect (sourceModel, &QAbstractItemModel::rowsAboutToBeInserted, this, [this](const QModelIndex &parent, int start, int end) {
        beginInsertRows(parent, start, end);
    });
    connect (sourceModel, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &parent, int start, int end) {
        endInsertRows();
    });
    DRoleMapProxyModel::setSourceModel(sourceModel);
}

QByteArray PlayItemTableProxyModel::getHeaderConfiguration() const {
    QByteArray a;
    QDataStream in(&a, QIODevice::WriteOnly);
    for (const HeaderData_t &data : qAsConst(headers)) {
        in << data.role;
        in << data.title;
        in << data.format;
    }
    return a;
}

void PlayItemTableProxyModel::setHeaderConfiguration(QByteArray &data) {
    if (data.isEmpty()) {
        // default
        QList<int> role_list = QList<int>{
                PlayItemModel::ItemPlaying, PlayItemModel::ItemArtistAlbum,
                PlayItemModel::ItemTrackNum, PlayItemModel::ItemTitle,
                PlayItemModel::ItemLength};
        for (int i : qAsConst(role_list)) {
            HeaderData_t header = getDefaultHeaderData(i);
            header.title = QString{};
            addHeader(header);
        }
    }
    else {
        qDebug() << "READ HEADER CONFIG";
        // TODO clean up current state;
        QDataStream in(&data, QIODevice::ReadOnly);
        HeaderData_t header;
        while (!in.atEnd()) {
            in >> header.role;
            in >> header.title;
            in >> header.format;
            addHeader(header);
        }
    }
}

void PlayItemTableProxyModel::setRoleMap(int idx, HeaderData_t &hd) {
    // todo format custom
    if (hd.role >= 0) {
        if (hd.role > PlayItemModel::PlayItemRoleLast) {
            PlayItemModel *m = qobject_cast<PlayItemModel*>(getPlayItemModel());
            if (!m) {
                custom_roles_queued.insert(idx, hd);
                return;
            }
            else {
                hd.role = m->addFormat(hd.format);
            }
        }
        if (hd.role == PlayItemModel::ItemPlaying) {
            setHeaderData(idx, Qt::Horizontal, PlayItemModel::ItemPlayingDecoration, Qt::DecorationRole);
            setHeaderData(idx, Qt::Horizontal, PlayItemModel::ItemPlaying, Qt::DisplayRole);
        }
        else {
            setHeaderData(idx, Qt::Horizontal, hd.role, Qt::DisplayRole);
            setHeaderData(idx, Qt::Horizontal, PlayItemModel::ItemEmpty, Qt::DecorationRole);
        }
    }

    if (!hd.title.isEmpty()) {
        setHeaderData(idx, Qt::Horizontal, hd.title, Qt::EditRole);
    }
    else {
        setHeaderData(idx, Qt::Horizontal, QString{}, Qt::EditRole);
    }
}

void PlayItemTableProxyModel::addHeader (HeaderData_t hd) {
    int idx =  headers.count();
    setRoleMap(idx, hd);
    headers.append(hd);
}
void PlayItemTableProxyModel::insertHeader (int idx, HeaderData_t hd) {
    // todo
    addHeader(hd);
}

void PlayItemTableProxyModel::replaceHeader (int idx, HeaderData_t hd) {
    if (idx >= 0 && idx < headers.count()) {
        headers.replace(idx, hd);
        setRoleMap(idx, hd);
    }
}

void PlayItemTableProxyModel::removeHeader (int idx) {
    // todo
    if (idx >= 0 && idx < headers.count()) {
        setHeaderData(idx, Qt::Horizontal, {}, Qt::DisplayRole);
        headers.removeAt(idx);
    }
}

HeaderData_t PlayItemTableProxyModel::getHeaderData(int idx) {
    if (idx >= 0 && idx < headers.count()) {
        return headers.at(idx);
    }
    return HeaderData_t{-1};
}

HeaderData_t PlayItemTableProxyModel::getDefaultHeaderData(int role) {
    return createHeader(role, PlayItemModel::defaultTitle(role),
                        PlayItemModel::defaultFormat(role));
}
