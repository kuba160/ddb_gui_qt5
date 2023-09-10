#ifndef PROXYPLAYITEMMODEL_H
#define PROXYPLAYITEMMODEL_H

#include <QIdentityProxyModel>

class ProxyPlayItemModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit ProxyPlayItemModel(QObject *parent = nullptr);
};

#endif // PROXYPLAYITEMMODEL_H
