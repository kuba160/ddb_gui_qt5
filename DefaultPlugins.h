#ifndef DEFAULTPLUGINS_H
#define DEFAULTPLUGINS_H

#include <vector>
#include "PluginLoader.h"

class DefaultPlugins {
public:
    DefaultPlugins();
    ~DefaultPlugins();
    DBWidgetInfo *WidgetReturn(unsigned long);
private:
    std::vector<DBWidgetInfo> *widgetLibrary;
    DBWidgetInfo volumeSlider,seekSlider, playbackButtons, artworkWidget, playlistWidget, tabBar, tabBarD, dummy, medialib;
    DBWidgetInfo playlistBrowser;
};

//}
#endif // DEFAULTPLUGINS_H
