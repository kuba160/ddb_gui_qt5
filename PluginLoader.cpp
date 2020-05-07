
#include <QObject>
#include <cstring>
#include "PluginLoader.h"
#include "QtGui.h"

#include <DBApi.h>
#include "MainWindow.h"

#include "DefaultPlugins.h"

extern MainWindow *w;
extern DBApi *api;

//namespace PluginLoader {

//    static std::vector<ExternalWidget_t> widgetLibrary;
//    static std::vector<LoadedWidget_t> widgetLibraryLoaded;

PluginLoader::PluginLoader(DBApi* Api) : QObject(nullptr), DBToolbarWidget (nullptr, Api) {
    widgetLibrary = new std::vector<ExternalWidget_t>();
    widgetLibraryLoaded = new std::vector<LoadedWidget_t>();
    actions = new std::vector<QAction *>();
    actions_create = new std::vector<QAction *>();
    toolbars = new std::vector<QToolBar *>();
/*
    DefaultPlugins df;
    unsigned long i = 0;
    ExternalWidget_t *df_widget;
    while ((df_widget = df.WidgetReturn(i))) {
        widgetLibraryAppend(df_widget);
        i++;
    }
    */
}

PluginLoader::~PluginLoader() {
    /*delete widgetLibrary;

    while(widgetLibraryLoaded->size()) {
        delete widgetLibraryLoaded->at(0).widget;
        widgetLibraryLoaded->erase(widgetLibraryLoaded->begin());
    }
    delete widgetLibraryLoaded;
*/
    // todo clean actions, actions create, toolbars
}

void PluginLoader::widgetLibraryAppend(DB_plugin_t *plugin, QWidget *(*constructor)(QWidget *, DBApi *)) {
    ExternalWidget_t temp;
    memcpy(&temp.plugin,plugin, sizeof(DB_plugin_t));
    temp.constructor = constructor;
    //widgetLibrary->push_back(temp);
}

void PluginLoader::widgetLibraryAppend(ExternalWidget_t *widget) {
    if (widget) {
     //   widgetLibrary->push_back(*widget);
    }
}
/*
long PluginLoader::widgetLibraryLoad(unsigned long num) {
    return 0;
    if (num>= widgetLibrary->size()) {
        return -1;
        //return nullptr;
    }
    ExternalWidget_t *p = &widgetLibrary->at(num);
    LoadedWidget_t temp;
    temp.header = p;

    // todo fix pointers
    temp.widget = p->constructor(nullptr, api);
    if (!temp.widget) {
        return -1;
    }
    widgetLibraryLoaded->push_back(temp);
    return widgetLibraryLoaded->size() - 1;
}*/

int PluginLoader::widgetLibraryAdd(QWidget *parent, unsigned long num) {
    // check if plugin of that name already exists
    // if not, create new one with name plugin_2
    // TODO

    // load
    long ret = -1;//widgetLibraryLoad(num);

    if (ret < 0) {
        return static_cast<int>(ret);
    }
    unsigned long loaded_num = static_cast<unsigned long>(ret);

    // plugin loaded, create action that shows it
    QAction *action = new QAction(widgetFriendlyName(loaded_num));
    QAction *action_create = new QAction(widgetFriendlyName(loaded_num));

    action->setCheckable(true);
    // todo: detect if enabled
    action->setChecked(true);

    action_create->setCheckable(false);

    connect(action, SIGNAL(toggled(bool)), this, SLOT(actionHandlerCheckable(bool)));
    connect(action_create, SIGNAL(triggered(bool)), this, SLOT(actionHandler(bool)));
    //actions->push_back(action);
    //actions_create->push_back(action_create);

    QToolBar *toolbar = new QToolBar(parent);
    toolbar->addWidget(widgetLibraryLoaded->at(loaded_num).widget);

    // get this from settings
    toolbar->setVisible(true);

    emit toolbarLoaded(toolbar);
    //toolbars->push_back(toolbar);
    //SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarStopped, "DeaDBeeF %_deadbeef_version%").toString().toUtf8().constData()
    //ToolbarStack[ToolbarStackCount] = new QToolBar(this);
    //ToolbarStack[ToolbarStackCount]->addWidget(pl->widgetLibraryLoad(i));
    //this->a5ddToolBar(ToolbarStack[ToolbarStackCount]);
    //QAction *action = ui->menuView->addAction()
    //action->setCheckable(true);
    // detect if enabled
    //action->setChecked(true);
    //connect(action,SIGNAL(ac))

    return 0;
}


void PluginLoader::actionHandlerCheckable(bool check) {
    // check with text();
    QObject *s = sender();

    unsigned long i;
    for (i = 0; i < actions->size(); i++) {
        if (actions->at(i) == s) {
            break;
        }
    }
    // show/hide toolbar with num=i
    //toolbars->at(i)->setVisible(check);
}

void PluginLoader::actionHandler(bool check) {
    // use sender() to detect which widget to act on
}

DBWidgetInfo *PluginLoader::widgetLibraryGetInfo(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    //ExternalWidget_t *p = &widgetLibrary->at(num);
    //return &p->info;
}


QString PluginLoader::widgetName(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    ExternalWidget_t *p = &widgetLibrary->at(num);
    return p->info.internalName;
}

QString PluginLoader::widgetFriendlyName(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    ExternalWidget_t *p = &widgetLibrary->at(num);
    return p->info.friendlyName;
}
