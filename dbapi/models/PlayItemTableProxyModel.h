#ifndef PLAYITEMTABLEPROXYMODEL_H
#define PLAYITEMTABLEPROXYMODEL_H

#include "DRoleMapProxyModel.h"

#include <QObject>

typedef struct HeaderData_s {
    int role;
    QString title;
    QString format;
} HeaderData_t;


class PlayItemTableProxyModel : public DRoleMapProxyModel
{
public:
    explicit PlayItemTableProxyModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *) override;

    QByteArray getHeaderConfiguration() const;
    void setHeaderConfiguration(QByteArray &);

    static HeaderData_t createHeader (int role, QString title = {}, QString format = {}) {
        return HeaderData_t{role, title, format};
    }

    void addHeader (HeaderData_t);
    void insertHeader (int idx, HeaderData_t);
    void replaceHeader (int idx, HeaderData_t);
    void removeHeader (int idx);

    HeaderData_t getHeaderData(int idx);
    HeaderData_t getDefaultHeaderData(int role);


protected:
    QList<HeaderData_t> headers;

    QHash<int, HeaderData_t> custom_roles_queued;

    void setRoleMap(int idx, HeaderData_t &);

    QAbstractItemModel *getPlayItemModel();
};

#endif // PLAYITEMTABLEPROXYMODEL_H
