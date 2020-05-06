
#include <cstring>
#include "PluginLoader.h"
#include "QtGui.h"

PluginLoader::PluginLoader (){

    //load_plugin (&qtCoverart);
}

int PluginLoader::load_plugin(QtPlugin_t *plugin) {
    std::memcpy (loaded+loaded_num,plugin, sizeof(QtPlugin_t));
    loaded_num++;

    return 0;
}

QtPlugin_t* PluginLoader::get_plugin (const char* name) {
    int i;
    for (i = 0; i < loaded_num; i++) {
        if (loaded[loaded_num].plugin.name && strcmp (loaded[loaded_num].plugin.name, name) == 0) {
            return &(loaded[loaded_num]);
        }
    }
    return nullptr;
}
