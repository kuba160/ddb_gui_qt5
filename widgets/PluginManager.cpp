#include "PluginManager.h"

#include <QToolBar>
#include <QDockWidget>
#include <QStatusBar>

#include <QQuickWidget>
#include <QQmlContext>
#include <QQuickItem>
#include <QThread>

#include <QQuickView>

#define DBAPI (this->api)
/*
enum DBWidgetType {
    TypeDummy           = 0,
    TypeToolbar         = 1<<0,
    TypeMainWidget      = 1<<1,
    TypeStatusBar       = 1<<2
};*/

DBWidget::DBWidget(QWidget *parent, DBApi *Api, PluginWidgetsWrapper &info, int i) {

    instance = i;
    // Tranlate friendly name and set up name with instance number
    QString friendlyName = info.tr(info.property("friendlyName"). toString().toUtf8().constData());
    QString internalName = info.property("internalName").toString();
    //QString internalName
    if (instance) {
        qDebug() << "qt5: DBWidget: Loading new instance of plugin" << friendlyName;
        //internalName = QString(QString("%1_%2") .arg(internalName) .arg(instance));
        friendlyName.append(" (%1)");
        friendlyName = friendlyName.arg(instance);
        internalName.append("_%1");
        internalName = internalName.arg(instance);

    }
    //setProperty("internalName", internalName);
    //setObjectName(internalName + "_wrapper");

    // true_parent (Toolbar/DockWidget/MainWindow)
    QString type = info.property("widgetType").toString();
    if (type == "toolbar")
        DB_parent = new QToolBar(friendlyName, parent); // remember parent
    else if (type == "main")
        DB_parent = new QDockWidget(friendlyName, parent);
    else if (type == "statusbar")
        DB_parent = nullptr;
    else {
        DB_parent = nullptr;
        qDebug() << "DBWidget" << friendlyName << ": couldn't choose appropiate parent for type=" << type;
    }

    if (DB_parent) {
        //DB_parent->setObjectName(internalName + "_tparent");
        DB_parent->setProperty("friendlyName", friendlyName);
        DB_parent->setProperty("internalName", internalName);
        DB_parent->setObjectName(internalName + "_DBP");
        //true_parent->setProperty("internalName", internalName);
    }

    //bool locked = api->confGetValue("PluginLoader", "designMode", false).toBool();
    QString widgetFormat = info.property("widgetFormat").toString().toLower();


    if (widgetFormat == "qml") {
        widget = createQmlWrapper(DB_parent, Api, info, instance);
    }
    else {//(widgetFormat == "c++") {
        widget = qobject_cast<QWidget*>(info.getConstructor()(DB_parent, Api));
    }

    if (type == "toolbar") {
       qobject_cast<QToolBar *>(DB_parent)->addWidget(widget);
       //qobject_cast<QToolBar *>(DB_parent)->setMovable(!locked);
    }
    else if (type == "main") {
       qobject_cast<QDockWidget *>(DB_parent)->setWidget(widget);
       DB_parent->show();
       //if (locked) {
       //    qobject_cast<QDockWidget *>(true_parent)->setFeatures(QDockWidget::NoDockWidgetFeatures);
       //}
    }

       //
    if (type == "main") {
        empty_titlebar_toolbar = new QWidget(DB_parent);
        empty_titlebar_toolbar->setVisible(false);
        //setMovable(!locked);
    }
    if (type == "statusbar") {
        DB_parent = widget;
    }
}

QWidget* DBWidget::createQmlWrapper(QWidget *DB_parent, DBApi *api, PluginQmlWrapper &info, int instance) {
    QUrl source = info.property("widgetUrl").toUrl();

    if (source.toString().contains("Shader", Qt::CaseInsensitive)) {
        QQuickView *widget = new QQuickView();//new QQuickWidget();
        widget->setResizeMode(QQuickView::SizeRootObjectToView);

        if (info.property("widgetType").toString() == "main") {
            qobject_cast<QDockWidget*>(DB_parent)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }

        widget->rootContext()->setContextProperty("_db_bg_override", true);
        widget->rootContext()->setContextProperty("_db_bg", QPalette().window().color());
        // Set API and load widget
        //widget->rootContext()->setContextProperty("app", app);
        widget->rootContext()->setContextProperty("api", api);
        widget->rootContext()->setContextProperty("playback", &api->playback);
        widget->rootContext()->setContextProperty("conf", &api->conf);
        widget->rootContext()->setContextProperty("eq", &api->eq);
        widget->rootContext()->setContextProperty("cover", &api->playlist);
        widget->rootContext()->setContextProperty("playlist", &api->playlist);
        if (DB_parent)
            DB_parent->setProperty("instance", instance);
        widget->rootContext()->setContextProperty("DB_parent", DB_parent);
        widget->rootContext()->setProperty("instance",instance);
        widget->rootContext()->setProperty("_db_do_not_load",false);

        widget->setSource(source);

        while (widget->status() != QQuickView::Ready) {
            QThread::msleep(50);
        }
        widget->rootObject()->setProperty("instance",instance);

        //QWidget *container = QWidget::createWindowContainer(view, this);
        return QWidget::createWindowContainer(widget, DB_parent);
    }

    //QQuickView *view = new QQuickView();
    //

    QQuickWidget *widget = new QQuickWidget();
    widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    // Allow resize
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // TODO allow specifying size policy (and size) through properties
    if (source.toString() == "qrc:/qml/VolumeSliderQuick.qml") {
        widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    }
    else if (source.toString() == "qrc:/qml/SeekSlider.qml") {
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        widget->setMaximumSize(1000, 50);
        //widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    widget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // 17 bars default (TODO hardcoded)
    widget->setMinimumWidth(76);
    widget->setMinimumHeight(28);

    // Transparency fix
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    widget->setWindowFlags(Qt::SplashScreen);
    widget->setAttribute(Qt::WA_AlwaysStackOnTop);
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setClearColor(Qt::transparent);
    widget->rootContext()->setContextProperty("overrideBg", false);
#else
    //widget->rootContext()->setContextProperty("overrideBg", true);
    // TODO fix transparency for qt 6
    //widget->setWindowFlags(Qt::SplashScreen);
    widget->setAttribute(Qt::WA_AlwaysStackOnTop);
    //widget->setAttribute(Qt::WA_TranslucentBackground);
    //widget->setClearColor(Qt::transparent);
#endif

    // Prefer native font rendering
    #if (QT_VERSION >= QT_VERSION_CHECK(5,10,0))
    widget->quickWindow()->setTextRenderType(QQuickWindow::NativeTextRendering);
    #endif

    if (info.property("widgetType").toString() == "main") {
        qobject_cast<QDockWidget*>(DB_parent)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    widget->rootContext()->setContextProperty("_db_bg_override", true);
    widget->rootContext()->setContextProperty("_db_bg", QPalette().window().color());
    // Set API and load widget
    //widget->rootContext()->setContextProperty("app", app);
    widget->rootContext()->setContextProperty("api", api);
    widget->rootContext()->setContextProperty("playback", &api->playback);
    widget->rootContext()->setContextProperty("conf", &api->conf);
    widget->rootContext()->setContextProperty("eq", &api->eq);
    widget->rootContext()->setContextProperty("cover", &api->playlist);
    widget->rootContext()->setContextProperty("playlist", &api->playlist);
    if (DB_parent)
        DB_parent->setProperty("instance", instance);
    widget->rootContext()->setContextProperty("DB_parent", DB_parent);
    widget->rootContext()->setProperty("instance",instance);
    widget->rootContext()->setProperty("_db_do_not_load",false);

    widget->setSource(source);

    while (widget->status() != QQuickWidget::Ready) {
        QThread::msleep(50);
    }
    widget->rootObject()->setProperty("instance",instance);
    return widget;
}


PluginManager::PluginManager(QObject *parent, DBApi *Api)
    : QObject{parent},
      loader(this)
{
    api = Api;
}

PluginManager::~PluginManager() {
    for (DBWidget *w : widgets.values()) {
        delete w;
    }
}

QWidget * PluginManager::loadNewInstance(QMainWindow *window, PluginWidgetsWrapper *info) {
    QString internalName = info->property("internalName").toString();
    int instance = widgets.count(internalName);
    DBWidget *widget = new DBWidget(window, api, *info, instance);
    widgets.insert(internalName, widget);
    QString widgetType = info->property("widgetType").toString();
    if (widgetType == "toolbar") {
        window->addToolBar(qobject_cast<QToolBar *>(widget->DB_parent));
    }
    else if (widgetType == "main") {
        window->addDockWidget(Qt::RightDockWidgetArea, qobject_cast<QDockWidget *>(widget->DB_parent));
    }
    else if (widgetType == "statusbar") {
        window->setStatusBar(qobject_cast<QStatusBar *>(widget->widget));
    }
    return widget->DB_parent;
}

QWidget * PluginManager::loadNewInstance(QMainWindow *window, QString name, QString style) {
    QObject * obj = loader.getWrapper(name,style);
    if (!obj) {
        qDebug() << "ERROR getting wrapper";
        return nullptr;
    }
    return loadNewInstance(window,(PluginWidgetsWrapper*)obj);
}

void PluginManager::restoreWidgets(QMainWindow *window, QString name) {

}
