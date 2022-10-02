#ifndef PLUGINMODEL_H
#define PLUGINMODEL_H

#include <QAbstractListModel>
#include <QObject>

#include <deadbeef/deadbeef.h>

class PluginModel : public QAbstractListModel {
    Q_OBJECT
    DB_functions_t* deadbeef;
public:
    explicit PluginModel(QObject *parent, DB_functions_t *deadbeef);

    enum PluginRoles {
        PluginName = Qt::UserRole,
        PluginId,
        PluginType,
        PluginApiMajor,
        PluginApiMinor,
        PluginVersionMajor,
        PluginVersionMinor,
        PluginDescr,
        PluginCopyright,
        PluginWebsite,
        //PluginPtr,
        PluginConf
    };
    Q_ENUM(PluginRoles)

    QHash<int, QByteArray> roleNames() const override;

    int rowCount (const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
public slots:
    void sort(int column, Qt::SortOrder sort = Qt::AscendingOrder) override;

private:
    QList<int> plugins_idx;
};

#endif // PLUGINMODEL_H
