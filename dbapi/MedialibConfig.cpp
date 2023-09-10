#include "MedialibConfig.h"

#include <QDebug>

MedialibConfig::MedialibConfig(QObject *parent, DB_mediasource_t *Plug,
                               ddb_mediasource_source_t *Source) : QObject(parent) {

    plug = Plug;
    if (strcmp(plug->plugin.id, "medialib")) {
        qDebug() << "MedialibConfig: plugin is not medialib!";
        return;
    }
    ddb_medialib_plugin_api_t *Api = static_cast<ddb_medialib_plugin_api_t*>((void*)plug->get_extended_api());
    ml_api = Api;
    source = Source;

    size_t count;
    char **folders = Api->get_folders(source, &count);
    for (int i = 0; i < count; i++) {
        m_folders.append(folders[i]);
    }
    Api->free_folders(source, folders, count); // why count?

}

QStringList MedialibConfig::getFolders() {
    return m_folders;
}

void MedialibConfig::setFolders(QStringList l) {
    const char** folders = (const char **) malloc(sizeof(char*) * l.count());
    for (int i = 0; i < l.count(); i++) {
        folders[i] = strdup(l[i].toUtf8().constData());
    }

    ml_api->set_folders(source, folders, l.count());

    for (int i = 0; i < l.count(); i++) {
        free((void*) folders[i]);
    }
    free(folders);

    emit foldersChanged();
}

void MedialibConfig::appendFolder(QString f) {
    ml_api->append_folder(source, f.toUtf8().constData());
    emit foldersChanged();
}
