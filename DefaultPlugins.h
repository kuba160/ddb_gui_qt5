#ifndef DEFAULTPLUGINS_H
#define DEFAULTPLUGINS_H

#include <vector>
#include "PluginLoader.h"

class DefaultPlugins {
public:
    DefaultPlugins();
    ~DefaultPlugins();
    void WidgetsInsert( int (*widgetLibraryAppend)(ExternalWidget_t *widget));
    ExternalWidget_t *WidgetReturn(unsigned long);
private:
    std::vector<ExternalWidget_t> *widgetLibrary;
};

//}
#endif // DEFAULTPLUGINS_H
