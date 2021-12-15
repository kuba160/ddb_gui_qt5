#ifndef DEFAULTPLUGINS_H
#define DEFAULTPLUGINS_H

#include <QObject>
#include <vector>
#include "DBApi.h"

class DefaultPlugins : public QObject {
    Q_OBJECT
public:
    DefaultPlugins(QObject *parent = nullptr);
    ~DefaultPlugins();
    DBWidgetInfo *WidgetReturn(unsigned long);
private:
    std::vector<DBWidgetInfo> *widgetLibrary;
    DBWidgetInfo volumeSlider,seekSlider, playbackButtons, artworkWidget,
                 playlistWidget, tabBar, tabBarD, dummy, medialib, logviewer,
                 queueManager, statusBar, volumeSliderQuick, seekSliderQuick,
                 tabBarQuick, playbackButtonsQuick;
    DBWidgetInfo playlistBrowser;
};

//}
#endif // DEFAULTPLUGINS_H
