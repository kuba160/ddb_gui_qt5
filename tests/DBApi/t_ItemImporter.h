#ifndef T_ITEMIMPORTER_H
#define T_ITEMIMPORTER_H

#include <QObject>
#include "dbapi/DBApi.h"

class t_ItemImporter : public QObject
{
    Q_OBJECT
public:
    explicit t_ItemImporter(DBApi *Api);

private:
    DBApi *api;

private slots:
    void addFolderTest();
    void addFileTest();
    void addPlaylistTest();

signals:
};

#endif // T_ITEMIMPORTER_H
