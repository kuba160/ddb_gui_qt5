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
    QWidget *(*constructor)(QWidget *, DBApi *);
} ExternalWidget_t;

typedef struct LoadedWidget_s{
    ExternalWidget_t *header;
    QWidget *widget;
    unsigned char instance;
    QString *friendlyName;
    QString *internalName;
} LoadedWidget_t;


class PluginLoader : public QObject, public DBToolbarWidget{
    Q_OBJECT
public:
    PluginLoader (DBApi *Api);
    ~PluginLoader();

    // Add new widget to database
    int widgetLibraryAppend(DBWidgetInfo *, QWidget *(*constructor)(QWidget *, DBApi *));
    int widgetLibraryAppend(ExternalWidget_t *widget);

    // load widget from database (internal, use add)
    // returns num of loaded widget
    long widgetLibraryLoad(unsigned long num);

    int widgetLibraryAdd(QWidget *parent, unsigned long num);
    int widgetLibraryAddInternal(QWidget *parent, unsigned long num);

    // get widget info from database
    DBWidgetInfo *widgetLibraryGetInfo(unsigned long num);

    // loaded widgets
    QString *widgetName(unsigned long num);
    QString *widgetFriendlyName(unsigned long num);

private:
    // list of widgets that can be added
    std::vector<ExternalWidget_t> *widgetLibrary;
    // actions that can create widgets from widget library (linked with above)
    std::vector<QAction *> *actions_create;


    // widgets currently shown (can have multiple copies, values etc.) (bad name)
    std::vector<LoadedWidget_t> *widgetLibraryLoaded;
    // toolbar currently shown (linked to widgets above)
    std::vector<QToolBar *> *toolbars;
    // actions that can show/hide widgets (linked with above)
    std::vector<QAction *> *actions;
    // actions that can destroy widgets
    std::vector<QAction *> *actions_destroy;




public slots:
    // handle widget actions (menu etc.)
    void actionHandlerCheckable(bool);
    void actionHandler(bool);
    // todo rename to conf-load or something
    void updateActionChecks();
    // conf-save
    void actionChecksSave();
    // context menu
    void contextMenuBuilder(QPoint &pos);

    //
    int addWidget(QWidget *parent, const QString *);
    //void widgetLoaded(unsigned long num);

    // get action New
    QAction *actionNewGet(unsigned long);

signals:
    void toolBarCreated(QToolBar *);
    void actionPluginCreated(QAction *);
    void actionPluginAddCreated(QAction *);
};

//}
#endif // PLUGINLOADER_H
