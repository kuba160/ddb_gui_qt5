#ifndef MEDIALIBCONFIG_H
#define MEDIALIBCONFIG_H

#include <QObject>
#include <deadbeef/deadbeef.h>
#include <medialib.h>
class MedialibConfig : public QObject {
    Q_OBJECT
public:
    MedialibConfig(QObject *parent, DB_mediasource_t *plug, ddb_mediasource_source_t *source);

    Q_PROPERTY(QStringList folders READ getFolders WRITE setFolders NOTIFY foldersChanged)

    QStringList getFolders();
    void setFolders(QStringList l);

    void appendFolder(QString f);

signals:
    void foldersChanged();

protected:
    DB_mediasource_t *plug;
    ddb_medialib_plugin_api_t *ml_api;
    ddb_mediasource_source_t *source;
    QStringList m_folders;
};

#endif // MEDIALIBCONFIG_H
