#include "PluginWidgetsLoader.h"
#include "DefaultPlugins.h"

PluginWidgetsWrapper::PluginWidgetsWrapper(QObject *parent, WidgetPluginConstructor c) : PluginQmlWrapper(parent) {

    constructor = c;
    QObject *obj = constructor(nullptr, nullptr);
    if (obj) {
        extractPluginInfo(obj);
        setProperty("widgetFormat", "widgets");
        delete obj;
    }
}

WidgetPluginConstructor PluginWidgetsWrapper::getConstructor() {
    return constructor;
}

PluginWidgetsLoader::PluginWidgetsLoader(QObject *parent) : PluginLoader(parent)
{

    insertWidgetPlugins(default_plugin_list);
}

void PluginWidgetsLoader::insertWidgetPlugins(QList<WidgetPluginConstructor> list) {
    for (WidgetPluginConstructor &c : list) {
        PluginWidgetsWrapper *w = new PluginWidgetsWrapper(this, c);
        QString name = w->property("internalName").toString();
        if (!name.isEmpty()) {
            pluginLibrary.insert(name, w);
        }
    }
}
