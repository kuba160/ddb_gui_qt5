#include "t_PlaylistManager.h"
#include <QTest>

#define PM (&api->playlist)

t_PlaylistManager::t_PlaylistManager(DBApi *Api)
    : QObject{Api}
{
    api = Api;
}
