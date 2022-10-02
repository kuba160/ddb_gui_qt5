#ifndef PLUGINWIDGETSLOADER_H
#define PLUGINWIDGETSLOADER_H

#include <QObject>

#include "../PluginLoader.h"
#include "../dbapi/DBApi.h"


class PluginWidgetsWrapper : public PluginQmlWrapper {
    Q_OBJECT
    WidgetPluginConstructor constructor;

public:
    PluginWidgetsWrapper(QObject *parent, WidgetPluginConstructor);
    WidgetPluginConstructor getConstructor();

};

class PluginWidgetsLoader : public PluginLoader
{
    Q_OBJECT
public:
    PluginWidgetsLoader(QObject *parent);

    void insertWidgetPlugins(QList<WidgetPluginConstructor>);



};

#endif // PLUGINWIDGETSLOADER_H
