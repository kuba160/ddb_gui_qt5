#include "PluginModel.h"

#define DBAPI (this->deadbeef)

PluginModel::PluginModel(QObject *parent, DB_functions_t* Api) :
    QAbstractListModel{parent} {
    deadbeef = Api;

}

QHash<int, QByteArray> PluginModel::roleNames() const {
    QHash<int, QByteArray> list;
    list.insert(PluginName, "PluginName");
    list.insert(PluginId,"PluginId");
    list.insert(PluginType,"PluginType");
    list.insert(PluginApiMajor,"PluginApiMajor");
    list.insert(PluginApiMinor,"PluginApiMinor");
    list.insert(PluginVersionMajor,"PluginVersionMajor");
    list.insert(PluginVersionMinor,"PluginVersionMinor");
    list.insert(PluginDescr,"PluginDescr");
    list.insert(PluginCopyright,"PluginCopyright");
    list.insert(PluginWebsite,"PluginWebsite");
    //list.insert(PluginPtr,"PluginPtr");
    list.insert(PluginConf, "PluginConf");
    return list;
}

int PluginModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    int count = 0;
    DB_plugin_t **list = DBAPI->plug_get_list();
    if (list) {
        while(list[count]) {
            count++;
        }
    }

    return count;
}

QVariant PluginModel::data(const QModelIndex &parent, int role) const {
    DB_plugin_t **list = DBAPI->plug_get_list();
    if (parent.row() > rowCount() || parent.row() < 0) {
        return QVariant();
    }
    int row = parent.row();
    if (plugins_idx.size()) {
        row = plugins_idx.at(row);
    }
    DB_plugin_t *plug = list[row];
    switch (role) {
        case Qt::DisplayRole:
        case PluginName:
            return QString(plug->name);
        case PluginId:
            return QString(plug->id);
        case PluginType:
            return plug->type;
        case PluginApiMajor:
            return plug->api_vmajor;
        case PluginApiMinor:
            return plug->api_vminor;
        case PluginVersionMajor:
            return plug->version_major;
        case PluginVersionMinor:
            return plug->version_minor;
        case PluginDescr:
            return plug->descr;
        case PluginCopyright:
            return plug->copyright;
        case PluginWebsite:
            return plug->website;
        //case PluginPtr:
        //   return (void *) plug;
        case PluginConf:
            return QString(plug->configdialog);
    }
    return QVariant();
}

void PluginModel::sort(int column, Qt::SortOrder sort) {
    beginResetModel();
    QHash<QString, int> plugins_map;
    QStringList plugins_names;
    plugins_idx.clear();
    DB_plugin_t **list = DBAPI->plug_get_list();
    int i = 0;
    if (list) {
        while(list[i]) {
            plugins_map.insert(list[i]->name, i);
            plugins_names.append(list[i]->name);
            i++;
        }
    }
    plugins_names.sort();
    while (plugins_names.size()) {
        QString curr;
        if (sort == Qt::AscendingOrder) {
            curr = plugins_names.takeFirst();
        }
        else {
            curr = plugins_names.takeLast();
        }
        plugins_idx.append(plugins_map.value(curr));
    }
    endResetModel();
}

