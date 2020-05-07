#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <vector>
#include <deadbeef/deadbeef.h>
#include <QtWidgets>
#include <QAction>
#include <QToolBar>
#include "DBApi.h"

extern DBApi *api;

typedef struct ExternalWidget_s{
    DBWidgetInfo info;
    DB_plugin_t plugin;
    QWidget *(*constructor)(QWidget *, DBApi *);
} ExternalWidget_t;

typedef struct LoadedWidget_s{
    ExternalWidget_t *header;
    QWidget *widget;
} LoadedWidget_t;


class PluginLoader : public QObject, public DBToolbarWidget{
    Q_OBJECT
public:
    PluginLoader (DBApi *Api);
    ~PluginLoader();

    // Add new widget to database
    void widgetLibraryAppend(DB_plugin_t *plugin, QWidget *(*constructor)(QWidget *, DBApi *));
    void widgetLibraryAppend(ExternalWidget_t *widget);

    // load widget from database (internal, use add)
    // returns num of loaded widget
    //long widgetLibraryLoad(unsigned long num);

    int widgetLibraryAdd(QWidget *parent, unsigned long num);

    // get widget info from database
    DBWidgetInfo *widgetLibraryGetInfo(unsigned long num);

    // loaded widgets
    QString widgetName(unsigned long num);
    QString widgetFriendlyName(unsigned long num);

private:
    std::vector<ExternalWidget_t> *widgetLibrary;
    std::vector<LoadedWidget_t> *widgetLibraryLoaded;

    std::vector<QAction *> *actions;
    std::vector<QAction *> *actions_create;

    std::vector<QToolBar *> *toolbars;

public slots:
    // handle widget actions (menu etc.)
    void actionHandlerCheckable(bool);
    void actionHandler(bool);
    //
    void widgetLoaded(unsigned long num);

signals:
    void toolbarLoaded(QToolBar *);
};

//}
#endif // PLUGINLOADER_H
