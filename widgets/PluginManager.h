#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMainWindow>
#include "PluginWidgetsLoader.h"
#include "PluginConfParser.h"

class DBWidget {
public:
    int instance;
    QWidget *empty_titlebar_toolbar;
    QWidget *DB_parent; // dynamic parent depending on context
    QWidget *widget;

    DBWidget(QWidget *parent, DBApi *Api, PluginWidgetsWrapper &info, int instance);

    QWidget *createQmlWrapper(QWidget *DB_parent, DBApi *Api, PluginQmlWrapper &info, int instance);
};



class PluginManager : public QObject
{
    Q_OBJECT
    DBApi * api;
public:
    explicit PluginManager(QObject *parent, DBApi *Api = nullptr);
    ~PluginManager();
    PluginWidgetsLoader loader;
    PluginConfParser parser;



    QWidget *loadNewInstance(QMainWindow *parent, QString name, QString style = QString());
    void removeInstance(QString name, int instance);

    void restoreWidgets(QMainWindow *window, QString name);

protected:
    QWidget *loadNewInstance(QMainWindow *parent, PluginWidgetsWrapper *info);

    QMultiHash<QString, DBWidget *> widgets;
signals:

};

#endif // PLUGINMANAGER_H
