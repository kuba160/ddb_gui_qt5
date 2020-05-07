#include "DefaultPlugins.h"

#include <vector>

#include "VolumeSlider.h"

DefaultPlugins::DefaultPlugins() {
    widgetLibrary = new std::vector<ExternalWidget_t>();

    ExternalWidget_t volumeSlider;
    volumeSlider.info.internalName = QString("volumeSlider");
    volumeSlider.info.friendlyName = QString("Volume Slider");
    volumeSlider.info.isToolbar = true;
    volumeSlider.constructor = VolumeSlider::constructor;
    //widgetLibrary->push_back(volumeSlider);

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

void DefaultPlugins::WidgetsInsert( void (*widgetLibraryAppend)(ExternalWidget_t *widget)) {
    std::vector<ExternalWidget_t>::iterator it;
    int i = 0;

    for(it = widgetLibrary->begin(); it != widgetLibrary->end(); it++,i++ ) {
        widgetLibraryAppend(&it[i]);
    }
};
