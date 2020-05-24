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
#include <QWindow>
#include <cstring>
#include "PluginLoader.h"
#include "QtGui.h"
#include "QtGuiSettings.h"

#include <DBApi.h>
#include "MainWindow.h"

#include "DefaultPlugins.h"

extern MainWindow *w;
extern DBApi *api;

PluginLoader::PluginLoader(DBApi* Api) : QObject(nullptr), DBToolbarWidget (nullptr, Api) {
    qDebug() << "qt5: PluginLoader initialized";
    // List of plugins that can be loaded
    widgetLibrary = new std::vector<ExternalWidget_t>();
    // List of plugins/widgets that are used
    loadedWidgets = new std::vector<LoadedWidget_t>();

    // Load internal plugins
    {
        DefaultPlugins df;
        unsigned long i = 0;
        DBWidgetInfo *info_toload;
        while ((info_toload = df.WidgetReturn(i))) {
            qDebug() << "qt5: PluginLoader:" << info_toload->internalName << "added to widgetLibrary";
            widgetLibraryAppend(info_toload);
            i++;
        }
    }
}

PluginLoader::~PluginLoader() {
    qDebug() << "qt5: PluginLoader cleaning";
    // THIS ONE IS A BIG TODO HERE
    // just clean after everything
    // cannot be that hard
    delete widgetLibrary;

    //while(widgetLibraryLoaded->size()) {
        // widgets are passed and adopted by mainwindow
        //delete widgetLibraryLoaded->at(0).widget;
        //widgetLibraryLoaded->erase(widgetLibraryLoaded->begin());
   // }
    //delete widgetLibraryLoaded;
    delete loadedWidgets;

    // todo clean actions, actions create, toolbars
}

void PluginLoader::RestoreWidgets(QWidget *parent) {
    QStringList slist = settings->getValue(QString("PluginLoader"),
                                           QString("PluginsLoaded"),
                                           QVariant(QStringList()
                                                    << QString("playbackButtons")
                                                    << QString("seekSlider")
                                                    << QString("volumeSlider"))).toStringList();
    int i;
    for (i = 0; i < slist.size(); i++) {
        pl->addWidget(parent, &slist.at(i));
    }
}

int PluginLoader::widgetLibraryAppend(DBWidgetInfo *info) {
    if (info && info->friendlyName.size() && info->internalName.size()) {
        ExternalWidget_t temp;
        temp.info = *info;
        // Create new action for creating that widget
        QAction *action_create = new QAction(info->friendlyName);
        action_create->setCheckable(false);
        connect(action_create, SIGNAL(triggered(bool)), this, SLOT(actionHandler(bool)));
        temp.actionCreateWidget = action_create;
        emit actionPluginAddCreated(action_create);
        // Add to library
        widgetLibrary->push_back(temp);
        widgetLibrarySort();
        return 0;
    }
    return -1;
}

void PluginLoader::widgetLibrarySort() {
    std::sort(widgetLibrary->begin(), widgetLibrary->end());
}

LoadedWidget_t * PluginLoader::widgetByNum(unsigned long num) {
    if (num < loadedWidgets->size()) {
        return &loadedWidgets->at(num);
    }
    return nullptr;
}

LoadedWidget_t * PluginLoader::widgetByName(QString *name) {
    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        if (loadedWidgets->at(i).internalName == *name) {
            return &loadedWidgets->at(i);
        }
    }
    return nullptr;
}

int PluginLoader::loadFromWidgetLibrary(unsigned long num) {
    if (num >= widgetLibrary->size()) {
        return -1;
    }

    ExternalWidget_t *p = &widgetLibrary->at(num);
    LoadedWidget_t temp;
    temp.header = p;
    temp.instance = getTotalInstances(p->info.internalName);
    temp.empty_titlebar_toolbar = nullptr;

    if (temp.instance) {
        qDebug() << "qt5: PluginLoader: Loading new instance of plugin" << p->info.internalName;
        temp.friendlyName = new QString(QString("%1 (%2)") .arg(p->info.friendlyName) .arg(temp.instance));
        temp.internalName = new QString(QString("%1_%2") .arg(p->info.internalName) .arg(temp.instance));
    }
    else {
        temp.friendlyName = new QString(p->info.friendlyName);
        temp.internalName = new QString(p->info.internalName);
    }

    qDebug() << "qt5: PluginLoader: Loading widget" << *temp.friendlyName;

    switch (p->info.type) {
    case DBWidgetInfo::TypeWidgetToolbar:
        temp.widget = p->info.constructor(nullptr, api);
        temp.toolbar = new QToolBar(nullptr);
        temp.toolbar->setObjectName(*temp.internalName);
        temp.toolbar->addWidget(temp.widget);
        temp.dockWidget = nullptr;
        break;
    case DBWidgetInfo::TypeToolbar:
        temp.widget = nullptr;
        temp.toolbar = p->info.constructorToolbar(nullptr, api);
        temp.toolbar->setObjectName(*temp.internalName);
        temp.dockWidget = nullptr;
        break;
    case DBWidgetInfo::TypeDockable:
        temp.widget = nullptr;
        temp.toolbar = nullptr;
        temp.dockWidget = p->info.constructorDockWidget(nullptr, api);
        temp.dockWidget->setObjectName(*temp.internalName);
        temp.dockWidget->setVisible(true);
        break;
    case DBWidgetInfo::TypeMainWidget:
        temp.widget = p->info.constructor(nullptr, api);
        temp.dockWidget = nullptr;
        mainWidget = temp.widget;
        break;
    default:
        qDebug() << "qt5: PluginLoader: Unknown widget type?";
        break;
    }

    // Action show/hide (check)
    temp.actionToggleVisible = new QAction(*temp.friendlyName);
    temp.actionToggleVisible->setCheckable(true);
    QString key = QString("%1/visible") .arg(*temp.internalName);
    bool isEnabled = settings->getValue(QString("PluginLoader"), key, QVariant(true)).toBool();
    temp.actionToggleVisible->setChecked(isEnabled);
    connect(temp.actionToggleVisible, SIGNAL(triggered(bool)), this, SLOT(actionHandlerCheckable(bool)));
    emit actionToggleVisibleCreated(temp.actionToggleVisible);

    // Action remove
    temp.actionDestroy = new QAction(*temp.friendlyName);
    temp.actionDestroy->setCheckable(false);
    connect(temp.actionDestroy, SIGNAL(triggered(bool)), this, SLOT(actionHandlerRemove(bool)));
    emit actionPluginRemoveCreated(temp.actionDestroy);

    if (p->info.type == DBWidgetInfo::TypeWidgetToolbar || p->info.type == DBWidgetInfo::TypeToolbar) {
        temp.toolbar->setVisible(isEnabled);
        emit toolBarCreated(temp.toolbar);
    }
    else if (p->info.type == DBWidgetInfo::TypeDockable) {
        temp.dockWidget->setVisible(isEnabled);
        if (temp.internalName != QString("playlist"))
            emit dockableWidgetCreated(temp.dockWidget);
    }

    // Expect our internal value to match reality
    // Widgets should be locked after config got loaded in MainWindow
    if (areWidgetsLocked) {
        switch (temp.header->info.type) {
        case DBWidgetInfo::TypeWidgetToolbar:
        case DBWidgetInfo::TypeToolbar:
              temp.toolbar->setMovable(false);
              break;
        case DBWidgetInfo::TypeDockable:
            if (temp.empty_titlebar_toolbar == nullptr) {
                temp.empty_titlebar_toolbar = new QWidget (temp.dockWidget);
            }
            temp.dockWidget->setTitleBarWidget(temp.empty_titlebar_toolbar);
            temp.dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
            break;
        default:
            qDebug() << "qt5: PluginLoader: Unknown widget type?";
        }
    }
    loadedWidgets->push_back(temp);
    return 0;
}

unsigned char PluginLoader::getTotalInstances(QString internalName) {
    unsigned char instance = 0;
    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        if (loadedWidgets->at(i).header->info.internalName.compare(internalName) == 0) {
            // same name
            instance = loadedWidgets->at(i).instance + 1;
        }
    }
    return instance;
}

void PluginLoader::removeWidget(unsigned long num) {
    if (num >= loadedWidgets->size()) {
        qDebug() <<"qt5: PluginLoader: removeWidget non existent?";
        return;
    }
    LoadedWidget_t *w = &loadedWidgets->at(num);
    switch (w->header->info.type) {
    case DBWidgetInfo::TypeWidgetToolbar:
        w->toolbar->setVisible(false);
        delete w->widget;
        delete w->toolbar;
        break;
    case DBWidgetInfo::TypeToolbar:
        w->toolbar->setVisible(false);
        delete w->toolbar;
        break;
    case DBWidgetInfo::TypeDockable:
        w->dockWidget->setVisible(false);
        delete w->dockWidget;
        break;
    default:
        qDebug() << "qt5: PluginLoader: Unknown widget type?";
        break;
    }
    w->actionDestroy->setVisible(false);
    delete w->actionDestroy;
    w->actionToggleVisible->setVisible(false);
    delete w->actionToggleVisible;
}

QWidget *PluginLoader::getMainWidget() {
    return mainWidget;
}

int PluginLoader::loadFromWidgetLibraryNew(unsigned long num) {
    int ret = loadFromWidgetLibrary(num);
    if (!ret) {
        QStringList slist = settings->getValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(QStringList())).toStringList();
        unsigned long num_loaded = loadedWidgets->size() - 1;

        slist.append(loadedWidgets->at(num_loaded).header->info.internalName);
        slist.sort();
        settings->setValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(slist));
    }
    return ret;
}


void PluginLoader::actionHandlerCheckable(bool check) {
    // check with text();
    QObject *s = sender();

    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        LoadedWidget_t *w = &loadedWidgets->at(i);
        if (w->actionToggleVisible == s) {
            // show/hide toolbar with num=i
            switch (w->header->info.type) {
            case DBWidgetInfo::TypeWidgetToolbar:
            case DBWidgetInfo::TypeToolbar:
                w->toolbar->setVisible(check);
                break;
            case DBWidgetInfo::TypeDockable:
                w->dockWidget->setVisible(check);
                break;
            default:
                qDebug() << "qt5: PluginLoader: Unknown widget type?";
                break;
            }
            QString key = QString("%1/visible") .arg(*w->internalName);
            settings->setValue(QString("PluginLoader"), key,check);
            return;
        }
    }
    // toolbar not found
    qDebug() << "qt5: PluginLoader: Widget toggled but no pointer available!" << endl;

}

void PluginLoader::actionHandler(bool check) {
    Q_UNUSED(check)
    QObject *s = sender();

    unsigned long i;
    for (i = 0; i < widgetLibrary->size(); i++) {
        if (widgetLibrary->at(i).actionCreateWidget == s) {
            // create new widget
            loadFromWidgetLibraryNew(i);
            return;
        }
    }
    // toolbar not found
    qDebug() << "qt5: PluginLoader: Widget could not be found, adding failed!" << endl;
}

void PluginLoader::actionHandlerRemove(bool check) {
    QObject *s = sender();

    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        LoadedWidget_t *wi = &loadedWidgets->at(i);
        if (wi->actionDestroy == s) {
            // destroy
            QStringList slist = settings->getValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(QStringList())).toStringList();
            int j = slist.indexOf(wi->header->info.internalName);
            if (j != -1) {
                slist.removeAt(j);
                slist.sort();
                settings->setValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(slist));

                settings->removeValue(QString("PluginLoader"), *wi->internalName);
            }
            removeWidget(i);
            loadedWidgets->erase(loadedWidgets->begin() + i);
            if (loadedWidgets->size() == 0) {
               w->windowViewActionRemoveToggleHide(false);
            }
            return;
        }
    }
    // toolbar not found
    qDebug() << "qt5: PluginLoader: Widget could not be found, removing failed!" << endl;
}

DBWidgetInfo *PluginLoader::widgetLibraryGetInfo(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    ExternalWidget_t *p = &widgetLibrary->at(num);
    return &p->info;
}


QString *PluginLoader::widgetName(unsigned long num) {
    if (num>= loadedWidgets->size()) {
        return nullptr;
    }
    LoadedWidget_t *p = &loadedWidgets->at(num);
    return p->internalName;
}

QString *PluginLoader::widgetFriendlyName(unsigned long num) {
    if (num>= loadedWidgets->size()) {
        return nullptr;
    }
    LoadedWidget_t *p = &loadedWidgets->at(num);
    return p->friendlyName;
}


void PluginLoader::updateActionChecks() {
    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        QString key = QString("%1/visible") .arg(*loadedWidgets->at(i).internalName);
        bool value = settings->getValue(QString("PluginLoader"), key, QVariant(true)).toBool();
        ///loadedWidgets->at(i).actionToggleVisible->setChecked(value);
        if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeWidgetToolbar || loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeToolbar) {
            loadedWidgets->at(i).toolbar->setVisible(value);
        }
    }
}

void PluginLoader::actionChecksSave() {
    // all widgets become invisible?

    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        QString key = QString("%1/visible") .arg(*loadedWidgets->at(i).internalName);
        if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeWidgetToolbar || loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeToolbar) {
            settings->setValue(QString("PluginLoader"), key,loadedWidgets->at(i).toolbar->isVisible());
        }
        else if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeDockable) {
            settings->setValue(QString("PluginLoader"), key,loadedWidgets->at(i).dockWidget->isVisible());
        }
    }

}

int PluginLoader::addWidget(QWidget *parent, const QString *name) {
    unsigned long i;
    for (i = 0; i < widgetLibrary->size(); i++) {
            if(name->compare(widgetLibrary->at(i).info.internalName) == 0) {
                int ret = loadFromWidgetLibrary(i);
                // set parent?
                // TODO
                return ret;
            }
    }
    return -1;
}

void PluginLoader::lockWidgets(bool lock) {
    areWidgetsLocked = lock;
    //for
    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeWidgetToolbar || loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeToolbar) {
            loadedWidgets->at(i).toolbar->setMovable(!lock);
        }
        else if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeDockable) {
            if (loadedWidgets->at(i).empty_titlebar_toolbar == nullptr) {
                loadedWidgets->at(i).empty_titlebar_toolbar = new QWidget(loadedWidgets->at(i).dockWidget);
            }
            if (lock) {
                loadedWidgets->at(i).dockWidget->setTitleBarWidget(loadedWidgets->at(i).empty_titlebar_toolbar);
                loadedWidgets->at(i).dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
            }
            else {
                loadedWidgets->at(i).dockWidget->setTitleBarWidget(nullptr);
                loadedWidgets->at(i).dockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
            }
            //loadedWidgets->at(i).dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        }

    }
}

QAction *PluginLoader::actionNewGet(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    return widgetLibrary->at(num).actionCreateWidget;
}

void PluginLoader::contextMenuBuilder(QPoint &pos) {
    // todo
    //QMenu *headerContextMenu = new QMenu();
    //headerContextMenu->addAction(QString("test"));
    //headerContextMenu->move(headerContextMenu->mapToGlobal(pos));
    //headerContextMenu->show();
}
