#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <deadbeef/deadbeef.h>
#include <QtWidgets>
#include <QAction>
#include <QQuickWidget>
#include <QQmlImageProviderBase>
#include "DBApi.h"
#include "DefaultPlugins.h"

extern DBApi *api;
extern QStringList default_plugins;

class PluginLoader;

class LoadedWidget : public QObject{
    Q_OBJECT
public:
    LoadedWidget(DBWidgetInfo &wi, PluginLoader *pl);
    ~LoadedWidget();

    void setMovable(bool state);
    void setMain(bool value);
    void setVisible(bool visible);
    // Default widget info (read-only)
    const DBWidgetInfo header;
    // instance of same plugin
    quint32 instance;

    // Properties:
    // friendlyName - Widget friendly name
    // internalName - Widget internal name

    // empty titlebar, used for dockable widgets, internal use only (to show/hide titlebar)
    QWidget *empty_titlebar_toolbar;
    QWidget *widget;

    // can be toolbar
    QWidget *true_parent;
};

// General Qt Quick widget encapsulation
class DBQuickWidget : public QQuickWidget, DBWidget {
    Q_OBJECT
public:
    DBQuickWidget(QWidget *parent, DBApi *api, QString source);
protected:
    void resizeEvent(QResizeEvent *) override;
};

class PluginLoader : public QObject{
    Q_OBJECT
public:
    PluginLoader ();
    ~PluginLoader();

    // loads all widgets from config
    // to be called by MainWindow
    void RestoreWidgets(QMainWindow *parent);
public:
    //// widgetLibrary
    //
    // Add new widget to database
    // Called by every external plugin (registerWidget function)
    void widgetLibraryAppend(DBWidgetInfo *);
protected:
    // get widget info from database
    DBWidgetInfo *widgetLibraryGet(int num);
    DBWidgetInfo *widgetLibraryGet(const QString);
    int widgetLibraryGetNum(const QString);

    //// loadedWidgets
    //
    // loads Widget from library
    // supports multiple instances
    int loadFromWidgetLibrary(int num);

public:
    // widgetLibrary (copy)
    QList<DBWidgetInfo *> getWidgetLibrary();
    // widgets (remember to free)
    QList<DBWidgetInfo>* getWidgets();
    // widget at
    DBWidgetInfo getWidgetAt(int num);
    DBWidgetInfo getWidget(QString internalName);
    // Main Widget list
    QStringList getMainWidgets();
    // get total amount of instances for a specific widget
    quint32 getTotalInstances(QString name);

    // get current mainwidget (internalName)
    QString getMainWidget();
private:
    // list of widgets that can be added
    QList<DBWidgetInfo *> widgetLibrary;
    // list of widgets that have been loaded
    QList<LoadedWidget *> loadedWidgets;
    // Default plugins list
    DefaultPlugins dp;
    // design mode on/off
    bool areWidgetsLocked;
    // current main widget selected
    LoadedWidget *mainWidget = nullptr;
    // pointer to mainWindow, if needed
    QMainWindow *mainWindow = nullptr;

    int statusBarCount = 0;

public slots:
    // load widget
    int addWidget(int num);
    int addWidget(QString internalName);
    // unload widget
    int removeWidget(int num);
    int removeWidget(QString internalName);

    // Sorts widgetLibrary in alphabetic order
    void widgetLibrarySort();
    // lock widgets toggle
    void setDesignMode(bool on);
    //
    int setMainWidget(QString internalName);
    //
    void setVisible(QString internalName, bool state);

signals:
    //
    void widgetAdded(int num);
    void widgetRemoved(QString);
    void widgetLibraryAdded(DBWidgetInfo i);
};

//}
#endif // PLUGINLOADER_H
