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
#include "PluginLoader.h"
#include "QtGui.h"
#include "QtGuiSettings.h"

#include <DBApi.h>
#include "MainWindow.h"

#include "DefaultPlugins.h"
#include "DeadbeefTranslator.h"

#define GETCONF(X,Y) confGetValue(QString("PluginLoader"),X,Y)
#define SETCONF(X,Y) confSetValue(QString("PluginLoader"),X,Y)

extern MainWindow *w;
extern DBApi *api;

QStringList default_plugins = { QString("playbackButtons"),
                                QString("seekSlider"),
                                QString("volumeSlider"),
                                QString("tabBar"),
                                QString("playlist"),
                                QString("statusbar")
                              };

LoadedWidget::LoadedWidget(DBWidgetInfo &info, PluginLoader *pl) : header(info) {
    instance = pl->getTotalInstances(info.internalName);

    // Tranlate friendly name and set up name with instance number
    QString friendlyName = tr(info.friendlyName.toUtf8());
    QString internalName;
    if (instance) {
        qDebug() << "qt5: PluginLoader: Loading new instance of plugin" << info.internalName;
        friendlyName = QString("%1 (%2)") .arg(friendlyName) .arg(instance);
        internalName = QString(QString("%1_%2") .arg(internalName) .arg(instance));
    }
    else {
        internalName = info.internalName;
    }
    setProperty("friendlyName", friendlyName);
    setProperty("internalName", internalName);
    setObjectName(internalName + "_wrapper");

    // true_parent (Toolbar/DockWidget/MainWindow)
    true_parent = info.type == DBWidgetInfo::TypeToolbar ? new QToolBar(friendlyName,w) :
                  info.type == DBWidgetInfo::TypeMainWidget ? new QDockWidget(friendlyName, w) :
                  info.type == DBWidgetInfo::TypeStatusBar ? nullptr :
                  new QWidget();

    if (true_parent) {
        true_parent->setObjectName(internalName + "_tparent");
        true_parent->setProperty("friendlyName", friendlyName);
        true_parent->setProperty("internalName", internalName);
    }

    bool locked = api->confGetValue("PluginLoader", "designMode", false).toBool();
    widget = info.constructor(true_parent,api);
    if (info.type == DBWidgetInfo::TypeToolbar) {
        qobject_cast<QToolBar *>(true_parent)->addWidget(widget);
        qobject_cast<QToolBar *>(true_parent)->setMovable(!locked);
    }
    else if (info.type == DBWidgetInfo::TypeMainWidget) {
        qobject_cast<QDockWidget *>(true_parent)->setWidget(widget);
        if (locked) {
            qobject_cast<QDockWidget *>(true_parent)->setFeatures(QDockWidget::NoDockWidgetFeatures);
        }
    }

    //
    if (info.type == DBWidgetInfo::TypeMainWidget) {
        empty_titlebar_toolbar = new QWidget(true_parent);
        setMovable(!locked);
    }

}

LoadedWidget::~LoadedWidget() {
    if (true_parent)
        delete true_parent;
}

void LoadedWidget::setMovable(bool on) {
    if (header.type == DBWidgetInfo::TypeToolbar) {
         qobject_cast<QToolBar *>(true_parent)->setMovable(on);
    }
    else if (!property("main").toBool() && header.type == DBWidgetInfo::TypeMainWidget) {
        if (on) {
            qobject_cast<QDockWidget *>(true_parent)->setTitleBarWidget(nullptr);
            qobject_cast<QDockWidget *>(true_parent)->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        }
        else {
            qobject_cast<QDockWidget *>(true_parent)->setTitleBarWidget(empty_titlebar_toolbar);
            qobject_cast<QDockWidget *>(true_parent)->setFeatures(QDockWidget::NoDockWidgetFeatures);
        }
    }
}

void LoadedWidget::setVisible(bool visible) {
    if (property("main").toBool()) {
        widget->setVisible(visible);
    }
    else if (true_parent) {
        true_parent->setVisible(visible);
    }
    if (header.type == DBWidgetInfo::TypeStatusBar && !visible) {
        delete widget;
    }
}

void LoadedWidget::setMain(bool state) {
    if (header.type == DBWidgetInfo::TypeMainWidget) {
        qobject_cast<QDockWidget *>(true_parent)->setWidget(state ? nullptr : widget);
        setProperty("main", state);
        true_parent->setVisible(!state);
    }
}

PluginLoader::PluginLoader() : QObject(nullptr) {
    qDebug() << "qt5: PluginLoader initialize:";

    // Load internal plugins
    {
        unsigned long i = 0;
        DBWidgetInfo *info_toload;
        while ((info_toload = dp.WidgetReturn(i))) {
            qDebug() << "qt5: PluginLoader:" << info_toload->internalName << "added to widgetLibrary";
            widgetLibrary.append(info_toload);
            i++;
        }
    }
    // External widgets will be appended to widgetLibrary in pluginConnect
}

PluginLoader::~PluginLoader() {

    qDebug() << "qt5: PluginLoader cleaning";
}

// widgetLibrary

void PluginLoader::widgetLibraryAppend(DBWidgetInfo *wi) {
    if (wi) {
        DBWidgetInfo *clone = new DBWidgetInfo(*wi);
        qDebug() << "qt5: PluginLoader:" << clone->internalName << "added to widgetLibrary";
        widgetLibrary.append(clone);
        emit widgetLibraryAdded(*clone);
    }
}

DBWidgetInfo* PluginLoader::widgetLibraryGet(const QString name) {
    int i;
    for (i = 0; i < widgetLibrary.length(); i++) {
        if(name.compare(widgetLibrary[i]->internalName) == 0) {
            return widgetLibrary[i];
        }
    }
    return nullptr;
}

int PluginLoader::widgetLibraryGetNum(const QString name) {
    int i;
    for (i = 0; i < widgetLibrary.length(); i++) {
        if(name.compare(widgetLibrary[i]->internalName) == 0) {
            return i;
        }
    }
    return -1;
}

DBWidgetInfo* PluginLoader::widgetLibraryGet(int num) {
    if (num < 0 || num >= widgetLibrary.length()) {
        return nullptr;
    }
    return widgetLibrary[num];
}

QList<DBWidgetInfo *> PluginLoader::getWidgetLibrary() {
    return QList<DBWidgetInfo *>(widgetLibrary);
}

QList<DBWidgetInfo>* PluginLoader::getWidgets() {
    QList<DBWidgetInfo> *list = new QList<DBWidgetInfo>;
    foreach(LoadedWidget *lw, loadedWidgets) {
        DBWidgetInfo wi;
        wi.friendlyName = lw->property("friendlyName").toString();
        wi.internalName = lw->property("internalName").toString();
        wi.type = lw->header.type;
        list->append(wi);
    }
    return list;
}

DBWidgetInfo PluginLoader::getWidgetAt(int num) {
    if (num >= 0 && num < loadedWidgets.length()) {
        DBWidgetInfo wi;
        wi.internalName = loadedWidgets.at(num)->property("internalName").toString();
        wi.friendlyName = loadedWidgets.at(num)->property("friendlyName").toString();
        wi.type = loadedWidgets.at(num)->header.type;
        // add if it is visible?
        return wi;
    }
    return DBWidgetInfo();
}

DBWidgetInfo PluginLoader::getWidget(QString internalName) {
    int i;
    for (i = 0; i < loadedWidgets.length(); i++) {
        if(internalName == loadedWidgets[i]->property("internalName").toString()) {
            return getWidgetAt(i);
        }
    }
    return DBWidgetInfo();
}


// widgets from widgetLibrary

void PluginLoader::RestoreWidgets(QMainWindow *parent) {
    if (api->GETCONF(QString("BuildDefaultLayout"),true).toBool()) {
        api->SETCONF("PluginsLoaded", default_plugins);
    }
    QStringList slist = api->GETCONF("PluginsLoaded", default_plugins).toStringList();
    mainWindow = parent;
    // Try to load widgets from config
    int i;
    for (i = 0; i < slist.size(); i++) {
        int num = widgetLibraryGetNum(slist[i]);
        if (num >= 0) {
            loadFromWidgetLibrary(num);
        }
        else {
            qDebug() << "qt5: PluginLoader: requested to load plugin " + slist.at(i) + " but it is missing from widget library!";
        }
    }
}

int PluginLoader::loadFromWidgetLibrary(int num) {
    // this method does not append widget into settings
    DBWidgetInfo *wi = widgetLibraryGet(num);
    if (wi) {
        // allow only one statusbar
        if (wi->type == DBWidgetInfo::TypeStatusBar && statusBarCount) {
            // todo display
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("Remove current statusbar first.");
            msgBox.exec();
            return -1;
        }
        LoadedWidget *lw = new LoadedWidget(*wi,this);
        if (lw->header.type == DBWidgetInfo::TypeToolbar) {
            if (api->GETCONF("BuildDefaultLayout", true).toBool() && lw->header.internalName == "tabBar") {
                mainWindow->addToolBarBreak();
                api->SETCONF("BuildDefaultLayout", false);
            }
            mainWindow->addToolBar(qobject_cast<QToolBar *>(lw->true_parent));
        }
        else if (lw->header.type == DBWidgetInfo::TypeStatusBar) {
            statusBarCount++;
            mainWindow->setStatusBar(qobject_cast<QStatusBar *>(lw->widget));
        }
        else {
            mainWindow->addDockWidget(Qt::RightDockWidgetArea,qobject_cast<QDockWidget *>(lw->true_parent));
            if (api->GETCONF("MainWidget","playlist").toString() == lw->property("internalName").toString()) {
                lw->setMain(true);
                mainWidget = lw;
                mainWindow->setCentralWidget(lw->widget);
            }
        }
        // visible
        bool visible = api->confGetValue("PluginLoader",
                          QString("%1/visible") .arg(lw->property("internalName").toString()),
                          true).toBool();
        lw->setVisible(visible);
        // movable
        lw->setMovable(api->GETCONF("designMode",false).toBool());
        loadedWidgets.append(lw);
        emit widgetAdded(loadedWidgets.length() -1);
        return loadedWidgets.length() -1;
    }
    return -1;
}

int PluginLoader::addWidget(int num) {
    // this metod DOES append widget into settings
    int ret = loadFromWidgetLibrary(num);
    if (ret >= 0) {
        QStringList slist = settings->getValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(QStringList())).toStringList();
        slist.append(loadedWidgets[ret]->header.internalName);
        slist.sort();
        api->SETCONF(QString("PluginsLoaded"),QVariant(slist));
    }
    return ret;
}

int PluginLoader::addWidget(QString internalName) {
    int ret = widgetLibraryGetNum(internalName);
    if (ret >= 0) {
        return addWidget(ret);
    }
    return -1;
}

int PluginLoader::removeWidget(int num) {
    if (num >= loadedWidgets.length()) {
        qDebug() <<"qt5: PluginLoader: removeWidget non existent?";
        return -1;
    }
    LoadedWidget *lw = loadedWidgets.takeAt(num);
    // Remove from config
    QStringList slist = settings->getValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(QStringList())).toStringList();
    for (int i = 0; i < slist.length(); i++) {
        QString str = slist.at(i);
        if (str == lw->header.internalName) {
            slist.takeAt(i);
            break;
        }
    }
    settings->setValue(QString("PluginLoader"), QString("PluginsLoaded"),slist);
    settings->remove(lw->property("internalName").toString());

    if (lw->header.type == DBWidgetInfo::TypeStatusBar) {
        w->setStatusBar(nullptr);
    }

    QString name = QString(lw->property("internalName").toString());
    delete lw;
    emit widgetRemoved(name);
    return 0;
}

int PluginLoader::removeWidget(QString internalName) {
    int i;
    for (i = 0; i < loadedWidgets.length(); i++) {
        if (loadedWidgets.at(i)->property("internalName").toString() == internalName) {
            return removeWidget(i);
        }
    }
    return -1;
}

int PluginLoader::setMainWidget(QString internalName) {
    int i;
    for (i = 0; i < loadedWidgets.length(); i++) {
        if (loadedWidgets.at(i)->property("internalName").toString() == internalName) {
            if (mainWidget) {
                mainWidget->setMain(false);
                mainWidget->setMovable(areWidgetsLocked);
            }
            mainWidget = loadedWidgets.at(i);
            mainWidget->setMain(true);
            mainWindow->setCentralWidget(mainWidget->widget);
            mainWidget->widget->setVisible(api->GETCONF(QString("%1/visible") .arg(mainWidget->property("internalName").toString()), true).toBool());
            api->SETCONF("MainWidget",loadedWidgets.at(i)->property("internalName"));
            return 0;
        }
    }
    return -1;
}

void PluginLoader::setVisible(QString internalName,bool state) {
    int i;
    for (i = 0; i < loadedWidgets.length(); i++) {
        if (loadedWidgets.at(i)->property("internalName").toString() == internalName) {
            loadedWidgets.at(i)->setVisible(state);
            api->confSetValue("PluginLoader",
                              QString("%1/visible") .arg(loadedWidgets.at(i)->property("internalName").toString()),
                              state);
            if (loadedWidgets.at(i)->header.type == DBWidgetInfo::TypeStatusBar) {
                if (state) {
                    LoadedWidget *lw = loadedWidgets.at(i);
                    lw->widget = lw->header.constructor(lw->true_parent,api);
                    w->setStatusBar(qobject_cast<QStatusBar *>(lw->widget));
                }
                else {
                    w->setStatusBar(nullptr);
                }
            }
        }
    }
}

void PluginLoader::widgetLibrarySort() {
    std::sort(widgetLibrary.begin(), widgetLibrary.end());
}

quint32 PluginLoader::getTotalInstances(QString name) {
    quint32 instance = 0;
    int i;
    for (i = 0; i < loadedWidgets.length(); i++) {
        if (loadedWidgets[i]->header.internalName.compare(name) == 0) {
            instance++;
        }
    }
    return instance;
}

QString PluginLoader::getMainWidget() {
    return mainWidget->property("internalName").toString();
}

QStringList PluginLoader::getMainWidgets() {
    QStringList list;
    foreach(LoadedWidget *lw, loadedWidgets) {
        list.append(lw->property("internalName").toString());
    }
    return list;
}

void PluginLoader::setDesignMode(bool on) {
    api->SETCONF("designMode", on);
    setProperty("designMode", on);
    foreach(LoadedWidget *wid, loadedWidgets) {
        wid->setMovable(on);
    }
}
