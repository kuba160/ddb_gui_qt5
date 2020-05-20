#include "DefaultPlugins.h"

#include <vector>

#include "VolumeSlider.h"
#include "SeekSlider.h"
#include "PlaybackButtons.h"

DefaultPlugins::DefaultPlugins() {
    widgetLibrary = new std::vector<ExternalWidget_t>();

    ExternalWidget_t volumeSlider;
    volumeSlider.info.internalName = QString("volumeSlider");
    volumeSlider.info.friendlyName = QString("Volume Slider");
    volumeSlider.info.isToolbar = true;
    volumeSlider.constructor = VolumeSlider::constructor;
    widgetLibrary->push_back(volumeSlider);

    ExternalWidget_t seekSlider;
    seekSlider.info.internalName = QString("seekSlider");
    seekSlider.info.friendlyName = QString("Seekbar");
    seekSlider.info.isToolbar = true;
    seekSlider.constructor = SeekSlider::constructor;
    widgetLibrary->push_back(seekSlider);

    ExternalWidget_t playbackButtons;
    playbackButtons.info.internalName = QString("playbackButtons");
    playbackButtons.info.friendlyName = QString("Playback Buttons");
    playbackButtons.info.isToolbar = true;
    playbackButtons.constructorToolbar = PlaybackButtons::constructor;
    widgetLibrary->push_back(playbackButtons);

}

DefaultPlugins::~DefaultPlugins() {
    delete widgetLibrary;
}

ExternalWidget_t *DefaultPlugins::WidgetReturn(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    return&widgetLibrary->at(num);
};

void DefaultPlugins::WidgetsInsert( int (*widgetLibraryAppend)(ExternalWidget_t *widget)) {
    std::vector<ExternalWidget_t>::iterator it;
    int i = 0;

    for(it = widgetLibrary->begin(); it != widgetLibrary->end(); it++,i++ ) {
        widgetLibraryAppend(&it[i]);
    }
};
