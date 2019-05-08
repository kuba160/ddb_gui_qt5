#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <deadbeef/deadbeef.h>
#include <QtWidgets>

typedef struct QtPlugin_s{
    DB_plugin_t plugin;
    QWidget *widget;
} QtPlugin_t;

class PluginLoader {
    QtPlugin_t loaded[32];
    int loaded_num = 0;
    PluginLoader();

public:
    int load_plugin (QtPlugin_t *);
    QtPlugin_t* get_plugin (const char* name);
};


#endif // PLUGINLOADER_H
