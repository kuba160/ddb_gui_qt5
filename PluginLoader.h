#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <vector>
#include <deadbeef/deadbeef.h>
#include <QtWidgets>
#include <QAction>
#include <QToolBar>
#include "DBApi.h"

extern DBApi *api;
extern QStringList default_plugins;

// Internal header for a plugin/widget
typedef struct ExternalWidget_s{
    // widget info passed from widget source
    DBWidgetInfo info;
    // action we created to create this widget
    QAction *actionCreateWidget;

    // operators used to sort widgets by name
    bool operator < (const ExternalWidget_s& str) const
        {
            return (info.friendlyName < str.info.friendlyName);
        }
    bool operator > (const ExternalWidget_s& str) const
        {
            return (info.friendlyName > str.info.friendlyName);
        }
} ExternalWidget_t;

typedef struct LoadedWidget_s{
    // Default widget info (read-only)
    ExternalWidget_t *header;
    // instance of same plugin
    unsigned char instance;
    // Widget friendly name (multiple instance support)
    QString *friendlyName;
    // Widget internal name (multiple instance support)
    QString *internalName;
    // Pointer to widget (if DBWidgetInfo::TypeWidgetToolbar)
    QWidget *widget;
    // Pointer to toolbar (if DBWidgetInfo::TypeWidgetToolbar or  DBWidgetInfo::TypeToolbar)
    QToolBar *toolbar;
    // Pointer to dockable widget (if DBWidgetInfo::TypeDockable)
    QDockWidget *dockWidget;
    // empty titlebar, used for dockable widgets
    QWidget *empty_titlebar_toolbar;
    // Action to make this widget visible
    QAction *actionToggleVisible;
    // Action to destroy this widget
    QAction *actionDestroy;
    // Action to select this widget as mainwidget
    QAction *actionMainWidget;

} LoadedWidget_t;


class PluginLoader : public QObject, public DBWidget{
    Q_OBJECT
public:
    PluginLoader (DBApi *Api);
    ~PluginLoader();

    //// widgetLibrary
    //
    // Add new widget to database
    // Called by every external plugin (registerWidget function)
    int widgetLibraryAppend(DBWidgetInfo *);
    // Sorts widgetLibrary in alphabetic order
    void widgetLibrarySort();
    // get widget info from database
    DBWidgetInfo *widgetLibraryGetInfo(unsigned long num);
    // get widget number by name
    unsigned long widgetLibraryGetNum(const QString *);

    //// loadedWidgets
    //
    // loads Widget from library
    // supports multiple instances
    int loadFromWidgetLibrary(unsigned long num);
    // same as above, but also adds widget to list of loaded widgets (in config)
    int loadFromWidgetLibraryNew(unsigned long num);
    // unload widget
    void removeWidget(unsigned long num);
    // loads all widgets from config
    // to be called by MainWindow
    void RestoreWidgets(QMainWindow *parent);


    //// Various widget functions
    //
    // get total amount of instances for a specific widget
    unsigned char getTotalInstances(QString name);
    // get total amount of mainwidget widgets (loaded)
    unsigned long getTotalMainWidgets();
    // get LoadedWidget_t by num
    LoadedWidget_t *widgetByNum(unsigned long num);
    // get LoadedWidget_t by name (multiple instances support)
    LoadedWidget_t *widgetByName(QString *);
    // get widget internal name
    QString *widgetName(unsigned long num);
    // get widget internal name from widget pointer
    QString *widgetName(void *pointer);
    // get widget friendly name
    QString *widgetFriendlyName(unsigned long num);
    //

    // get current mainwidget (can be nullptr)
    QWidget *getMainWidget();
    // when mainwidget changes
    void setMainWidget(LoadedWidget_t *);
    // ?
    void setMainWindow(QMainWindow *);

private:
    // list of widgets that can be added
    std::vector<ExternalWidget_t> *widgetLibrary;
    // list of widgets that have been loaded
    std::vector<LoadedWidget_t> *loadedWidgets;
    // design mode on/off
    bool areWidgetsLocked;
    // current main widget selected
    QWidget *mainWidget = nullptr;
    // pointer to mainWindow, if needed
    QMainWindow *mainWindow = nullptr;

public slots:
    // Show/Hide widget handler
    void actionHandlerCheckable(bool);
    // New widget handler
    void actionHandler(bool);
    // Delete widget handler
    void actionHandlerRemove(bool);
    // Main Widget chooser handler
    void actionHandlerMainWidget(bool);
    // slightly misleading name
    // synchronizes Show/Hide actions together with widgets
    void updateActionChecks();
    // save information if the widgets are visible
    void actionChecksSave();
    // context menu TODO
    void contextMenuBuilder(QPoint &pos);
    // lock widgets toggle
    void lockWidgets(bool lock);


    // get action New
    QAction *actionNewGet(unsigned long);

signals:
    // toolbar has been created by PluginLoader, required link to MainWindow
    void toolBarCreated(QToolBar *);
    // dockable widget has been created by PluginLoader, required link to MainWindow
    void dockableWidgetCreated(QDockWidget *);
    // widget has been created by PluginLoader, required to add action to View
    void actionToggleVisibleCreated(QAction *);
    // widget has been registered, add to menu necessary
    void actionPluginAddCreated(QAction *);
    //
    void actionPluginRemoveCreated(QAction *);
    // mainwidget has been registered
    void actionPluginMainWidgetCreated(QAction *);

    // there are no plugins loaded, hide "Remove..." option
    void loadedWidgetsEmpty();
    //
    void centralWidgetChanged(QWidget *);
};

//}
#endif // PLUGINLOADER_H
