#ifndef ITEMIMPORTER_H
#define ITEMIMPORTER_H

#include <QObject>
#include <QFuture>

#include <deadbeef/deadbeef.h>
#include <PlayItemIterator.h>


class ItemImporter {
public:
    static QFuture<PlayItemIterator> runFileImport(QStringList files);
    static QFuture<PlayItemIterator> runFolderImport(QStringList folders);
    static QFuture<PlayItemIterator> runPlaylistImport(QStringList playlist_files);
};


#endif // ITEMIMPORTER_H
