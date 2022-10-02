#include "WidgetLibraryModel.h"

WidgetLibraryModel::WidgetLibraryModel(QObject *parent, QMultiMap<QString, QObject*> *Map)
    : QAbstractListModel{parent} {
    map = Map;
}

QHash<int, QByteArray> WidgetLibraryModel::roleNames() const {
    return QHash<int, QByteArray>{
        {WidgetFriendlyName, "WidgetFriendlyName"},
        {WidgetInternalName, "WidgetInternalName"},
        {WidgetStyles, "WidgetStyles"},
        {WidgetTypes, "WidgetTypes"},
        {WidgetFormats, "WidgetFormats"},
        {WidgetQmlUrls, "WidgetQmlUrls"}
    };
}

int WidgetLibraryModel::rowCount (const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return map->uniqueKeys().count();
    }
    return 0;

}

QVariant WidgetLibraryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() >= 0 && index.row() < rowCount()) {
        QStringList keys = map->uniqueKeys();
        QString key = keys[index.row()];
        QObject *value = map->value(key);
        switch (role) {
            case Qt::DisplayRole:
            case WidgetFriendlyName:
                return value->property("friendlyName");
            case WidgetInternalName:
                return value->property("internalName");
            case WidgetStyles:
            case WidgetFormats:
            case WidgetTypes:
            case WidgetQmlUrls:
                QList<QObject *> values = map->values(key);
                const char *prop = "";
                switch (role) {
                    case WidgetStyles:
                        prop = "widgetStyle";
                        break;
                    case WidgetFormats:
                        prop = "widgetFormat";
                        break;
                    case WidgetTypes:
                        prop = "widgetType";
                        break;
                }
                if (values.count() == 1) {
                    if (role == WidgetQmlUrls) {
                        return value->property("widgetUrl");
                    }
                    return QStringList(value->property(prop).toString());
                }
                else {
                    QStringList names;

                    for (QObject *value : values) {
                        if (role == WidgetQmlUrls) {
                            QString url = value->property("widgetUrl").toString();
                            if (!url.isEmpty())
                                names.append(url);
                        }
                        else {
                            names.append(value->property(prop).toString());
                        }
                    }
                    return names;
                }
        }
    }
    return QVariant();
}
