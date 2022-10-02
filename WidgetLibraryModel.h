#ifndef WIDGETLIBRARYMODEL_H
#define WIDGETLIBRARYMODEL_H

#include <QAbstractListModel>
#include <QMultiMap>

class WidgetLibraryModel : public QAbstractListModel {
    Q_OBJECT
    QMultiMap<QString, QObject*> *map;
public:
    WidgetLibraryModel(QObject *parent, QMultiMap<QString, QObject*> *Map);

    enum WidgetLibraryNames {
        WidgetFriendlyName = Qt::UserRole,
        WidgetInternalName,
        WidgetStyles,
        WidgetTypes,
        WidgetFormats,
        WidgetQmlUrls
    };
    Q_ENUMS(WidgetLibraryNames)

    QHash<int, QByteArray> roleNames() const override;
    int rowCount (const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index = QModelIndex(), int role = Qt::DisplayRole) const override;
};

#endif // WIDGETLIBRARYMODEL_H
