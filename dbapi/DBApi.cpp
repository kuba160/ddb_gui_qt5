#include "DBApi.h"

#include <QDebug>

DBApi *DBApi_main = nullptr;

DBApi::DBApi(QObject *parent, DB_functions_t *Api) :
    QObject(parent),
    actions(this, Api),
    conf(this, Api),
    //cover(this, Api),
    eq(this, Api),
    playback(this, Api),
    playlist(this, Api) {
    deadbeef = Api;
    DBApi_main = this;
    //playlist.setProperty("_dbapi_cover_cache", QVariant::fromValue(&cover));

}

DBApi::~DBApi() {

}


int DBApi::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    playback.pluginMessage(id,ctx,p1,p2);
    playlist.pluginMessage(id,ctx,p1,p2);
    //cover.pluginMessage(id,ctx,p1,p2);
    return 0;
}
