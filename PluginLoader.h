#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <QObject>
#include <QUrl>
#include <QAbstractItemModel>

class WidgetLibraryModel;

class PluginQmlWrapper : public QObject {
    Q_OBJECT
public:
    PluginQmlWrapper(QObject *parent, QUrl source = QUrl());

    // Properties:
    // friendlyName - Widget friendly name
    // internalName - Widget internal name
    // todo add more

protected:
    void extractPluginInfo(QObject *obj);
};

class PluginLoader : public QObject {
    Q_OBJECT
    WidgetLibraryModel *m_plugins;
public:
    PluginLoader (QObject *parent);
    ~PluginLoader();


    Q_PROPERTY(QAbstractItemModel *list READ pluginLibraryModel CONSTANT)
    QAbstractItemModel *pluginLibraryModel();
    void insertPlugins(QStringList);

    //WidgetPluginConstructor getConstructor(QString &internalName, QString style = QString());
    QUrl getConstructorUrl(QString &internalName, QString style = QString());
    QObject *getWrapper(QString &internalName, QString style = QString(), QString type = QString());
protected:
    //// widgetLibrary
    ///
    //

    // list of widgets that can be added
    //QList<PluginWrapper *> widgetLibrary;
    QMultiMap<QString, QObject *> pluginLibrary;
};

//}
#endif // PLUGINLOADER_H
