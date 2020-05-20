
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

//namespace PluginLoader {

//    static std::vector<ExternalWidget_t> widgetLibrary;
//    static std::vector<LoadedWidget_t> widgetLibraryLoaded;

PluginLoader::PluginLoader(DBApi* Api) : QObject(nullptr), DBToolbarWidget (nullptr, Api) {
    qDebug() << "qt5: PluginLoader initialized" <<endl;
    widgetLibrary = new std::vector<ExternalWidget_t>();
    widgetLibraryLoaded = new std::vector<LoadedWidget_t>();
    actions = new std::vector<QAction *>();
    actions_create = new std::vector<QAction *>();
    toolbars = new std::vector<QToolBar *>();

    DefaultPlugins df;
    unsigned long i = 0;
    ExternalWidget_t *df_widget;
    while ((df_widget = df.WidgetReturn(i))) {
        qDebug() << "qt5: PluginLoader: " << df_widget->info.internalName << " added to widgetLibrary" << endl;
        widgetLibraryAppend(df_widget);
        i++;
    }
}

PluginLoader::~PluginLoader() {
    qDebug() << "qt5: PluginLoader cleaning" << endl;
// TODO??
    //this->actionChecksSave();
    delete widgetLibrary;

    while(widgetLibraryLoaded->size()) {
        // widgets are passed and adopted by mainwindow
        //delete widgetLibraryLoaded->at(0).widget;
        widgetLibraryLoaded->erase(widgetLibraryLoaded->begin());
    }
    delete widgetLibraryLoaded;

    // todo clean actions, actions create, toolbars
}

int PluginLoader::widgetLibraryAppend(DBWidgetInfo *info, QWidget *(*constructor)(QWidget *, DBApi *)) {
    if (info && info->friendlyName.size() && info->internalName.size()) {
        ExternalWidget_t temp;
        memcpy(&temp.info, info, sizeof(DBWidgetInfo));
        temp.constructor = constructor;
        widgetLibrary->push_back(temp);
        {
            QAction *action_create = new QAction(info->friendlyName);
            action_create->setCheckable(false);
            connect(action_create, SIGNAL(triggered(bool)), this, SLOT(actionHandler(bool)));
            actions_create->push_back(action_create);
            emit actionPluginAddCreated(action_create);
        }
        return 0;
    }
    return -1;
}

int PluginLoader::widgetLibraryAppend(ExternalWidget_t *widget) {
    if (widget) {
        if (widget->info.friendlyName.size() && widget->info.internalName.size()) {
            widgetLibrary->push_back(*widget);
            QAction *action_create = new QAction(widget->info.friendlyName);
            action_create->setCheckable(false);
            connect(action_create, SIGNAL(triggered(bool)), this, SLOT(actionHandler(bool)));
            actions_create->push_back(action_create);
            emit actionPluginAddCreated(action_create);
            return 0;
        }
    }
    return -1;
}

long PluginLoader::widgetLibraryLoad(unsigned long num) {
    if (num >= widgetLibrary->size()) {
        return -1;
        //return nullptr;
    }
    ExternalWidget_t *p = &widgetLibrary->at(num);

    // check if such widget already loaded
    unsigned char instance = 0;
    unsigned long i;
    for (i = 0; i < widgetLibraryLoaded->size(); i++) {
        if (widgetLibraryLoaded->at(i).header->info.internalName.compare(p->info.internalName) == 0) {
            // same name
            instance = widgetLibraryLoaded->at(i).instance + 1;
        }
    }
    LoadedWidget_t temp;
    temp.header = p;
    temp.instance = instance;
    if (instance) {
        qDebug() << "qt5: PluginLoader: Loading new instance of plugin" << p->info.internalName;
       // QString apx_internal = QString("_%1") .arg(instance);
        QString apx_friendly = QString(" (%1)") .arg(instance);
        //p->info.internalName.append(apx_internal);
        temp.friendlyName = new QString(QString("%1 (%2)") .arg(p->info.friendlyName) .arg(instance));
        temp.internalName = new QString(QString("%1_%2") .arg(p->info.internalName) .arg(instance));
    }
    else {
        temp.friendlyName = new QString(p->info.friendlyName);
        temp.internalName = new QString(p->info.internalName);
    }

    qDebug() << "qt5: PluginLoader: Loading widget " << *temp.friendlyName << endl;


    // todo fix pointers
    if (!p->info.toolbarConstructor) {
        // kinda bad fix
        // toolbar gonna load in widgetlibraryadd
        temp.widget = p->constructor(nullptr, api);
        if (!temp.widget) {
            return -1;
        }
    }
    else {
        if (!p->constructorToolbar) {
            // TODO leak error - temp.fname and iname are leaked
            return -1;
        }
        temp.widget = nullptr;
    }
    widgetLibraryLoaded->push_back(temp);
    return widgetLibraryLoaded->size() - 1;
}

int PluginLoader::widgetLibraryAdd(QWidget *parent, unsigned long num) {

    // load
    long ret = widgetLibraryLoad(num);

    if (ret < 0) {
        return static_cast<int>(ret);
    }
    unsigned long loaded_num = static_cast<unsigned long>(ret);

    QStringList slist = settings->getValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(QStringList())).toStringList();
    slist.append(widgetLibraryLoaded->at(loaded_num).header->info.internalName);
    settings->setValue(QString("PluginLoader"), QString("PluginsLoaded"),QVariant(slist));


    // Toolbar placeholder for widget
    QToolBar *toolbar;
    if (widgetLibraryLoaded->at(ret).header->info.toolbarConstructor) {
        toolbar = widgetLibraryLoaded->at(ret).header->constructorToolbar (nullptr, api);
    }
    else {
        toolbar = new QToolBar(parent);
        toolbar->addWidget(widgetLibraryLoaded->at(loaded_num).widget);
    }

    toolbar->setObjectName(*widgetLibraryLoaded->at(loaded_num).internalName);

    // plugin loaded, create action that shows it
    QAction *action = new QAction(*widgetFriendlyName(loaded_num));
    action->setCheckable(true);
    // added by user, always visible
    action->setChecked(true);

    connect(action, SIGNAL(toggled(bool)), this, SLOT(actionHandlerCheckable(bool)));
    actions->push_back(action);
    emit actionPluginCreated(action);

    // get this from settings
    toolbar->setVisible(true);
    toolbars->push_back(toolbar);
    emit toolBarCreated(toolbar);



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

int PluginLoader::widgetLibraryAddInternal(QWidget *parent, unsigned long num) {

    // load
    long ret = widgetLibraryLoad(num);

    if (ret < 0) {
        return static_cast<int>(ret);
    }
    unsigned long loaded_num = static_cast<unsigned long>(ret);

    // Toolbar placeholder for widget
    QToolBar *toolbar;
    if (widgetLibraryLoaded->at(ret).header->info.toolbarConstructor) {
        toolbar = widgetLibraryLoaded->at(ret).header->constructorToolbar (nullptr, api);
    }
    else {
        toolbar = new QToolBar(parent);
        toolbar->addWidget(widgetLibraryLoaded->at(loaded_num).widget);
    }
    toolbar->setObjectName(*widgetLibraryLoaded->at(loaded_num).internalName);
    //toolbar->addWidget(widgetLibraryLoaded->at(loaded_num).widget);

    // plugin loaded, create action that shows it
    QAction *action = new QAction(*widgetFriendlyName(loaded_num));
    action->setCheckable(true);
    // todo: detect if enabled
    action->setChecked(toolbar->isVisible());

    // todo: custom menu
    //toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolbar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(headerContextMenuBuilder(QPoint)));

    connect(action, SIGNAL(toggled(bool)), this, SLOT(actionHandlerCheckable(bool)));
    actions->push_back(action);
    emit actionPluginCreated(action);

    // get this from settings
    //toolbar->setVisible(true);
    toolbars->push_back(toolbar);
    emit toolBarCreated(toolbar);



    return 0;
}



void PluginLoader::actionHandlerCheckable(bool check) {
    // check with text();
    QObject *s = sender();

    unsigned long i;
    for (i = 0; i < actions->size(); i++) {
        if (actions->at(i) == s) {
            // show/hide toolbar with num=i
            toolbars->at(i)->setVisible(check);
            QString key = QString("%1.visible") .arg(*widgetLibraryLoaded->at(i).internalName);
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
    for (i = 0; i < actions_create->size(); i++) {
        if (actions_create->at(i) == s) {
            // create new widget
            widgetLibraryAdd(nullptr, i);
            return;
        }
    }
    // toolbar not found
    qDebug() << "qt5: PluginLoader: Widget could not be found, adding failed!" << endl;
}

DBWidgetInfo *PluginLoader::widgetLibraryGetInfo(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    ExternalWidget_t *p = &widgetLibrary->at(num);
    return &p->info;
}


QString *PluginLoader::widgetName(unsigned long num) {
    if (num>= widgetLibraryLoaded->size()) {
        return nullptr;
    }
    LoadedWidget_t *p = &widgetLibraryLoaded->at(num);
    return p->internalName;
}

QString *PluginLoader::widgetFriendlyName(unsigned long num) {
    if (num>= widgetLibraryLoaded->size()) {
        return nullptr;
    }
    LoadedWidget_t *p = &widgetLibraryLoaded->at(num);
    return p->friendlyName;
}


void PluginLoader::updateActionChecks() {
    unsigned long i;
    for (i = 0; i < actions->size(); i++) {
            QString key = QString("%1.visible") .arg(*widgetLibraryLoaded->at(i).internalName);
            bool value = settings->getValue(QString("PluginLoader"), key, QVariant(true)).toBool();
            actions->at(i)->setChecked(value);
            toolbars->at(i)->setVisible(value);
    }
}

void PluginLoader::actionChecksSave() {
    // all widgets become invisible?

    unsigned long i;
    for (i = 0; i < actions->size(); i++) {
            QString key = QString("%1.visible") .arg(*widgetLibraryLoaded->at(i).internalName);
            settings->setValue(QString("PluginLoader"), key,toolbars->at(i)->isVisible());
    }
}

int PluginLoader::addWidget(QWidget *parent, const QString *name) {
    unsigned long i;
    for (i = 0; i < widgetLibrary->size(); i++) {
            if(name->compare(widgetLibrary->at(i).info.internalName) == 0) {
                return widgetLibraryAddInternal(parent, i);
            }
    }
    return -1;
}

void PluginLoader::lockWidgets(bool lock) {
    //for
    unsigned long i;
    for (i = 0; i < widgetLibraryLoaded->size(); i++) {
        toolbars->at(i)->setMovable(!lock);
    }
}

QAction *PluginLoader::actionNewGet(unsigned long num) {
    if (num>= actions_create->size()) {
        return nullptr;
    }
    return actions_create->at(num);
}

void PluginLoader::contextMenuBuilder(QPoint &pos) {
    // todo
    //QMenu *headerContextMenu = new QMenu();
    //headerContextMenu->addAction(QString("test"));
    //headerContextMenu->move(headerContextMenu->mapToGlobal(pos));
    //headerContextMenu->show();
}
