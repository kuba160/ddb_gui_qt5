#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <vector>
#include <deadbeef/deadbeef.h>
#include <QtWidgets>
#include <QAction>
#include <QToolBar>
#include "DBApi.h"

extern DBApi *api;

//typedef DBWidgetInfo ExternalWidget_t;

typedef struct ExternalWidget_s{
    // widget info passed from widget source
    DBWidgetInfo info;
    // action we created to create this widget
    QAction *actionCreateWidget;
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

} LoadedWidget_t;


class PluginLoader : public QObject, public DBToolbarWidget{
    Q_OBJECT
public:
    PluginLoader (DBApi *Api);
    ~PluginLoader();

    // Add new widget to database
    // Called by every external plugin
    int widgetLibraryAppend(DBWidgetInfo *);

    // internal, create widget
    int loadFromWidgetLibrary(unsigned long num);

    // new plugin, add new to settings to load at startup
    int loadFromWidgetLibraryNew(unsigned long num);

    // get total amount of specific widget
    unsigned char getTotalInstances(QString name);

    //
    void removeWidget(unsigned long num);


    // get widget info from database
    DBWidgetInfo *widgetLibraryGetInfo(unsigned long num);

    // loaded widgets
    LoadedWidget_t *widgetByNum(unsigned long num);
    QString *widgetName(unsigned long num);
    QString *widgetFriendlyName(unsigned long num);

    void RestoreWidgets(QWidget *parent);

private:
    // list of widgets that can be added
    std::vector<ExternalWidget_t> *widgetLibrary;

    std::vector<LoadedWidget_t> *loadedWidgets;

    bool areWidgetsLocked;

public slots:
    // handle widget actions (menu etc.)
    void actionHandlerCheckable(bool);
    void actionHandler(bool);
    void actionHandlerRemove(bool);
    // todo rename to conf-load or something
    void updateActionChecks();
    // conf-save
    void actionChecksSave();
    // context menu
    void contextMenuBuilder(QPoint &pos);

    // add widget from name (without adding to PluginsLoaded)
    int addWidget(QWidget *parent, const QString *);
    //void widgetLoaded(unsigned long num);
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
    // there are no plugins loaded, hide "Remove..." option
    void loadedWidgetsEmpty();
};

//}
#endif // PLUGINLOADER_H
