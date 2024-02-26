/*
    PluginLoader - Dynamic Widget loader/manager
    Copyright (C) 2019-2020 Jakub Wasylków <kuba_160@protonmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <QObject>
#include <QDebug>
#include <QQmlContext>
#include <QQuickItem>
#include <QDir>
#include "PluginLoader.h"
#include <QQmlEngine>
#include <QThread>

#include "WidgetLibraryModel.h"

PluginQmlWrapper::PluginQmlWrapper(QObject *parent, QUrl source) : QObject(parent) {
    if (source.isValid()) {
        QQmlEngine engine;
        QQmlComponent component(&engine, source);
        engine.rootContext()->setProperty("instance", -1);
        engine.rootContext()->setContextProperty("_db_bg_override", true);
        engine.rootContext()->setContextProperty("_db_bg", "transparent");
        engine.rootContext()->setContextProperty("_db_do_not_load", true);
        QObject *plugin = component.create();
        if (plugin) {
            while(!component.isReady()) {
                QThread::usleep(1000);
            }
            PluginQmlWrapper::extractPluginInfo(plugin);
        }
        delete plugin;
        setProperty("widgetFormat", "qml");
        setProperty("widgetUrl", source);
    }
}

void PluginQmlWrapper::extractPluginInfo(QObject *plugin) {
    setProperty("friendlyName", plugin->property("friendlyName"));
    setProperty("internalName", plugin->property("internalName"));
    setProperty("widgetStyle", plugin->property("widgetStyle"));
    setProperty("widgetType", plugin->property("widgetType"));
    //setProperty("widgetFormat", plugin->property("widgetFormat"));
}

PluginLoader::PluginLoader(QObject *parent) : QObject(parent) {
    qDebug() << "qt5: PluginLoader initialize:";

    // Load local Qt Quick widgets (TODO load from deadbeef dir)
    QStringList default_qml_list;
    for (QString &str : QDir(":/qt/qml/DeaDBeeF/Q/GuiCommon").entryList()) {
        qDebug() << "Found built-in qml widget" << str;
        // TODO FIX
#if USE_WIDGETS
       default_qml_list.append(QString("qrc:/qt/qml/DeaDBeeF/Q/GuiCommon/").append(str));
#endif
    }
    insertPlugins(default_qml_list);

    m_plugins = new WidgetLibraryModel(this, &pluginLibrary);
    // External widgets will be appended to widgetLibrary in pluginConnect
}

QAbstractItemModel* PluginLoader::pluginLibraryModel() {
    return m_plugins;
}

void PluginLoader::insertPlugins(QStringList list) {
    for(const QString &path: std::as_const(list)) {
        if (!path.isEmpty()) {
            PluginQmlWrapper *plugin = new PluginQmlWrapper(this, path);
            QString internalName = plugin->property("internalName").toString();
            if (!internalName.isEmpty()) {
                pluginLibrary.insert(internalName, plugin);
            }
            else {
                delete plugin;
            }
        }
    }
}

PluginLoader::~PluginLoader() {
    qDebug() << "qt5: PluginLoader cleaning";

    delete m_plugins;
    m_plugins = nullptr;

    for(QObject *wrapper : std::as_const(pluginLibrary)) {
        delete wrapper;
    }
    pluginLibrary.clear();
}

QUrl PluginLoader::getConstructorUrl(QString &internalName, QString style) {
    QObject *wrapper = getWrapper(internalName, style, "qml");
    if (wrapper) {
        return wrapper->property("widgetUrl").toUrl();
    }
    return QUrl();
}

QObject * PluginLoader::getWrapper(QString &internalName, QString style, QString type) {
    auto [i, end] = pluginLibrary.equal_range(internalName);
    while (i != end) {
        if (!style.isEmpty()) {
            QString widgetStyle = i.value()->property("widgetStyle").toString();
            if (widgetStyle != style) {
                i++;
                continue;
            }
        }
        if (!type.isEmpty()) {
            QString widgetType = i.value()->property("widgetType").toString();
            if (widgetType != type) {
                i++;
                continue;
            }
        }
        return i.value();
     }
    return nullptr;
}


