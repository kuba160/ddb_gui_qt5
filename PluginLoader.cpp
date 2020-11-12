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

#define GETCONF(X,Y) settings->getValue(QString("PluginLoader"),X,Y)
#define SETCONF(X,Y) settings->setValue(QString("PluginLoader"),X,Y)

extern MainWindow *w;
extern DBApi *api;

QStringList default_plugins = { QString("playbackButtons"),
                                QString("seekSlider"),
                                QString("volumeSlider"),
                                QString("tabBar"),
                                QString("playlist") };

PluginLoader::PluginLoader(DBApi* Api) : QObject(nullptr), DBWidget (nullptr, Api) {
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
    // External widgets will be appended to widgetLibrary
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

void PluginLoader::RestoreWidgets(QMainWindow *parent) {
    QStringList slist = GETCONF("PluginsLoaded", default_plugins).toStringList();
    bool deflayout = GETCONF(QString("BuildDefaultLayout"),QVariant(true)).toBool();
    if (deflayout) {
        slist = default_plugins;
    }
    mainWindow = parent;
    // Try to load widgets from config
    int i;
    for (i = 0; i < slist.size(); i++) {
        unsigned long wl_num = widgetLibraryGetNum(&slist.at(i));
        if (wl_num != static_cast<unsigned long>(-1)) {
            pl->loadFromWidgetLibrary(wl_num);
        }
        else {
            qDebug() << "qt5: PluginLoader: requested to load plugin" + slist.at(i) + "but it is missing from widget library!";
        }
    }
}

int PluginLoader::widgetLibraryAppend(DBWidgetInfo *info) {
    if (info && info->friendlyName.size() && info->internalName.size()) {
        ExternalWidget_t temp;
        temp.info = *info;
        temp.info.friendlyName = dbtr->translate(nullptr, temp.info.friendlyName.toUtf8());
        // Create new action for creating that widget
        QAction *action_create = new QAction(temp.info.friendlyName);
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

unsigned long PluginLoader::widgetLibraryGetNum(const QString *name) {
    unsigned long i;
    for (i = 0; i < widgetLibrary->size(); i++) {
            if(name->compare(widgetLibrary->at(i).info.internalName) == 0) {
                return i;
            }
    }
    return static_cast<unsigned long>(-1);
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

    QString frname = dbtr->translate(nullptr, p->info.friendlyName.toUtf8());
    if (temp.instance) {
        qDebug() << "qt5: PluginLoader: Loading new instance of plugin" << p->info.internalName;
        temp.friendlyName = new QString(QString("%1 (%2)") .arg(frname) .arg(temp.instance));
        temp.internalName = new QString(QString("%1_%2") .arg(p->info.internalName) .arg(temp.instance));
    }
    else {
        temp.friendlyName = new QString(frname);
        temp.internalName = new QString(p->info.internalName);
    }

    qDebug() << "qt5: PluginLoader: Loading widget" << *temp.friendlyName;

    temp.actionMainWidget = nullptr;
    QWidget dummyName;
    switch (p->info.type) {
    case DBWidgetInfo::TypeToolbar:
        temp.toolbar = new QToolBar(mainWindow);
        temp.toolbar->setObjectName(*temp.internalName);
        temp.widget = p->info.constructor(temp.toolbar, api);
        temp.toolbar->addWidget(temp.widget);
        temp.dockWidget = nullptr;
        break;
    case DBWidgetInfo::TypeMainWidget:
        dummyName.setObjectName(*temp.internalName);
        temp.actionMainWidget = new QAction(*temp.friendlyName);
        temp.actionMainWidget->setCheckable(true);
        // if widget of TypeMainWidget is not selected as main widget, make it as dockwidget
        if (temp.internalName->compare(settings->getValue(QString("PluginLoader"), QString("MainWidget"), QString("playlist")).toString()) == 0 || \
                QString("").compare(settings->getValue(QString("PluginLoader"), QString("MainWidget"), QString("")).toString()) == 0) {
            temp.widget = p->info.constructor(&dummyName, api);
            temp.widget->setParent(mainWindow);
            temp.dockWidget = nullptr;
            setMainWidget(&temp);
            temp.actionMainWidget->setChecked(true);

        }
        else {
            temp.widget = p->info.constructor(&dummyName, api);
            temp.dockWidget = new QDockWidget(*temp.friendlyName, mainWindow);
            temp.dockWidget->setObjectName(*temp.internalName);
            temp.dockWidget->setWidget(temp.widget);
            temp.actionMainWidget->setChecked(false);
        }
        // Add widget to list
        connect(temp.actionMainWidget, SIGNAL(triggered(bool)), this, SLOT(actionHandlerMainWidget(bool)));
        emit actionPluginMainWidgetCreated(temp.actionMainWidget);
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

    if (p->info.type == DBWidgetInfo::TypeToolbar) {
        temp.toolbar->setVisible(isEnabled);
        // HACK FOR DEFAULT LAYOUT CREATION
        QVariant deflayout = settings->QSGETCONF(QString("BuildDefaultLayout"),QVariant(true));
        if (deflayout.toBool() && (temp.internalName == QString("tabBar"))){
            mainWindow->addToolBarBreak();
            settings->QSSETCONF(QString("BuildDefaultLayout"), QVariant(false));
        }
        emit toolBarCreated(temp.toolbar);
    }
    else if (p->info.type == DBWidgetInfo::TypeMainWidget && temp.dockWidget) {
        emit dockableWidgetCreated(temp.dockWidget);
        temp.dockWidget->setVisible(isEnabled);
    }

    // Expect our internal value to match reality
    // Widgets should be locked after config got loaded in MainWindow
    if (areWidgetsLocked) {
        switch (temp.header->info.type) {
        case DBWidgetInfo::TypeToolbar:
              temp.toolbar->setMovable(false);
              break;
        case DBWidgetInfo::TypeMainWidget:
            if (temp.dockWidget) {
                if (temp.empty_titlebar_toolbar == nullptr) {
                    temp.empty_titlebar_toolbar = new QWidget (temp.dockWidget);
                }
                temp.dockWidget->setTitleBarWidget(temp.empty_titlebar_toolbar);
                temp.dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
            }
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

unsigned long PluginLoader::getTotalMainWidgets() {
    unsigned long i;
    unsigned long amount = 0;
    for (i = 0; i < loadedWidgets->size(); i++) {
        if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeMainWidget) {
            amount++;
        }
    }
    return amount;
}

void PluginLoader::removeWidget(unsigned long num) {
    if (num >= loadedWidgets->size()) {
        qDebug() <<"qt5: PluginLoader: removeWidget non existent?";
        return;
    }
    LoadedWidget_t *lw = &loadedWidgets->at(num);
    switch (lw->header->info.type) {
    case DBWidgetInfo::TypeToolbar:
        lw->toolbar->setVisible(false);
        delete lw->widget;
        delete lw->toolbar;
        break;
    case DBWidgetInfo::TypeMainWidget:
        if (lw->dockWidget) {
            lw->dockWidget->setVisible(false);
            delete lw->widget;
            delete lw->dockWidget;
        }
        else {
            lw->widget->setVisible(false);
            if (!lw->dockWidget) {
                // replace current main widget
                unsigned long i;
                LoadedWidget_t *wr = nullptr;
                for (i = 0; i < loadedWidgets->size(); i++) {
                    wr = &loadedWidgets->at(i);
                    if (wr->header->info.type == DBWidgetInfo::TypeMainWidget) {
                        break;
                    }
                }
                setMainWidget(wr);
            }
            delete lw->widget;
        }
        w->windowViewActionMainWidget(nullptr);
        break;
    default:
        qDebug() << "qt5: PluginLoader: Unknown widget type?";
        break;
    }
    lw->actionDestroy->setVisible(false);
    delete lw->actionDestroy;
    lw->actionToggleVisible->setVisible(false);
    delete lw->actionToggleVisible;
    if (lw->actionMainWidget) {
        lw->actionMainWidget->setVisible(false);
        delete lw->actionMainWidget;
    }
}

QWidget *PluginLoader::getMainWidget() {
    return mainWidget;
}

void PluginLoader::setMainWidget(LoadedWidget_t *lw) {
    if (lw == nullptr) {
        //w->main_widgets->setVisible(false);
        w->setCentralWidget(nullptr);
        settings->QSSETCONF(QString("MainWidget"),QString(""));
        return;
    }

    if (lw->widget && !lw->dockWidget && lw->widget != mainWidget) {
        // probably main widget initialization on startup, make it easy
        //w->setCentralWidget(lw->widget);
        mainWidget = lw->widget;
        if (settings->getValue(QString("PluginLoader"), QString("MainWidget"),QString("")).toString().compare(lw->internalName))
            settings->QSSETCONF(QString("MainWidget"),QString(*lw->internalName));
        emit centralWidgetChanged(lw->widget);
        return;
    }
    if (lw->widget == mainWidget) {
        return;
    }
    // convert current main widget to dock
    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        LoadedWidget_t *lw_c = &loadedWidgets->at(i);
        if (lw_c->widget == mainWidget) {
            lw_c->dockWidget = new QDockWidget(*lw_c->friendlyName, w);
            lw_c->widget->setParent(lw_c->dockWidget);
            w->setCentralWidget(nullptr);
            lw_c->dockWidget->setWidget(lw_c->widget);
            lw_c->widget->setVisible(true);
            if (areWidgetsLocked) {
                if (!lw_c->empty_titlebar_toolbar)
                    lw_c->empty_titlebar_toolbar = new QWidget(lw_c->dockWidget);
                loadedWidgets->at(i).dockWidget->setTitleBarWidget(lw_c->empty_titlebar_toolbar);
                loadedWidgets->at(i).dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
            }
            else {
                loadedWidgets->at(i).dockWidget->setTitleBarWidget(nullptr);
                loadedWidgets->at(i).dockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
            }
            w->windowAddDockable(lw_c->dockWidget);
            break;
        }
    }
    // convert new widget to simple widget and remove its dock
    lw->widget->setParent(w);
    // segfault?
    lw->dockWidget->setWidget(nullptr);
    lw->dockWidget->setTitleBarWidget(nullptr);
    delete lw->empty_titlebar_toolbar;
    lw->empty_titlebar_toolbar = nullptr;
    delete lw->dockWidget;
    lw->dockWidget = nullptr;

    mainWidget = lw->widget;
    w->setCentralWidget(lw->widget);
    lw->actionMainWidget->setChecked(true);

    settings->QSSETCONF(QString("MainWidget"),QString(*lw->internalName));
    emit centralWidgetChanged(lw->widget);
}

void PluginLoader::setMainWindow(QMainWindow *mw) {
    mainWindow = mw;
}

int PluginLoader::loadFromWidgetLibraryNew(unsigned long num) {
    int ret = loadFromWidgetLibrary(num);
    if (!ret) {
        QStringList slist = settings->getValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(QStringList())).toStringList();
        unsigned long num_loaded = loadedWidgets->size() - 1;

        slist.append(loadedWidgets->at(num_loaded).header->info.internalName);
        slist.sort();
        settings->QSSETCONF(QString("PluginsLoaded"),QVariant(slist));
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
            case DBWidgetInfo::TypeToolbar:
                w->toolbar->setVisible(check);
                break;
            case DBWidgetInfo::TypeMainWidget:
                if (w->dockWidget) {
                    w->dockWidget->setVisible(check);
                }
                else {
                    w->widget->setVisible(check);
                }
                break;
            default:
                qDebug() << "qt5: PluginLoader: Unknown widget type?";
                break;
            }
            QString key = QString("%1/visible") .arg(*w->internalName);
            settings->QSSETCONF(key,check);
            return;
        }
    }
    // toolbar not found
    qDebug() << "qt5: PluginLoader: Widget toggled but no pointer available!" << ENDL;

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
    qDebug() << "qt5: PluginLoader: Widget could not be found, adding failed!" << ENDL;
}

void PluginLoader::actionHandlerRemove(bool check) {
    Q_UNUSED(check);
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
                settings->QSSETCONF(QString("PluginsLoaded"),QVariant(slist));

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
    qDebug() << "qt5: PluginLoader: Widget could not be found, removing failed!" << ENDL;
}

void PluginLoader::actionHandlerMainWidget(bool check) {
    Q_UNUSED(check);
    QObject *s = sender();

    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        LoadedWidget_t *wi = &loadedWidgets->at(i);
        if (wi->actionMainWidget == s) {
            if (wi->widget != mainWidget) {
                // new main widget requested
                return setMainWidget(wi);
            }
            else {
                return;
            }
        }
    }
    // toolbar not found
    qDebug() << "qt5: PluginLoader: Widget could not be found, changing mainwidget failed!" << ENDL;
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

QString *PluginLoader::widgetName(void *pointer) {
    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        LoadedWidget_t *lwt = &loadedWidgets->at(i);
        switch(lwt->header->info.type) {
        case DBWidgetInfo::TypeMainWidget:
            if (lwt->dockWidget && lwt->dockWidget == pointer) {
                return lwt->internalName;
            }
            break;
        case DBWidgetInfo::TypeToolbar:
            if (lwt->widget && lwt->widget == pointer) {
                return lwt->internalName;
            }
            else if (lwt->toolbar && lwt->toolbar == pointer) {
                return lwt->internalName;
            }
            break;
        case DBWidgetInfo::TypeDummy:
            // has no widget
            break;
        }
    }
    return nullptr;
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
        QVariant s = settings->QSGETCONF(QString("%1/visible") .arg(*loadedWidgets->at(i).internalName), QVariant(true));
        bool value = s.toBool();
        ///loadedWidgets->at(i).actionToggleVisible->setChecked(value);
        if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeToolbar) {
            loadedWidgets->at(i).toolbar->setVisible(value);
        }
        else if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeMainWidget) {
            if (loadedWidgets->at(i).dockWidget)
                loadedWidgets->at(i).dockWidget->setVisible(value);
            else {
                loadedWidgets->at(i).widget->setVisible(value);
            }
        }
    }
}

void PluginLoader::actionChecksSave() {
    // all widgets become invisible?
    qDebug() << "PluginLoader: saving widget status";

    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        QString key = QString("%1/visible") .arg(*loadedWidgets->at(i).internalName);
        if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeToolbar) {
            settings->QSSETCONF(key,loadedWidgets->at(i).toolbar->isVisible());
        }
        else if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeMainWidget) {
            if (loadedWidgets->at(i).dockWidget) {
                settings->QSSETCONF(key,loadedWidgets->at(i).dockWidget->isVisible());
            }
            else {
                settings->QSSETCONF(key,loadedWidgets->at(i).widget->isVisible());
            }
        }
    }
}

void PluginLoader::lockWidgets(bool lock) {
    areWidgetsLocked = !lock;
    lock = !lock;
    //for
    unsigned long i;
    for (i = 0; i < loadedWidgets->size(); i++) {
        if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeToolbar) {
            loadedWidgets->at(i).toolbar->setMovable(!lock);
        }
        else if (loadedWidgets->at(i).header->info.type == DBWidgetInfo::TypeMainWidget && loadedWidgets->at(i).dockWidget) {
            if (loadedWidgets->at(i).empty_titlebar_toolbar == nullptr) {
                loadedWidgets->at(i).empty_titlebar_toolbar = new QWidget(loadedWidgets->at(i).dockWidget);
            }
            if (lock) {
                loadedWidgets->at(i).dockWidget->setTitleBarWidget(loadedWidgets->at(i).empty_titlebar_toolbar);
                loadedWidgets->at(i).dockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
                //loadedWidgets->at(i).dockWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
               // if (loadedWidgets->at(i).dockWidget->isVisible()) {
                    loadedWidgets->at(i).dockWidget->setFixedWidth(loadedWidgets->at(i).dockWidget->width());
                    loadedWidgets->at(i).dockWidget->setFixedHeight(loadedWidgets->at(i).dockWidget->height());
                //}
            }
            else {
                loadedWidgets->at(i).dockWidget->setTitleBarWidget(nullptr);
                loadedWidgets->at(i).dockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
                loadedWidgets->at(i).dockWidget->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
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
