#include "DBApi.h"

#include <QDebug>
#include "ActionsDefault.h"


DBApi *DBApi_main = nullptr;
DB_functions_t *DBApi_deadbeef = nullptr;

DBApi::DBApi(QObject *parent, DB_functions_t *Api) :
    QObject(parent),
    actions(this),
    conf(this, Api),
    //cover(this, Api),
    eq(this, Api),
    playback(this, Api),
    playlist(this, Api),
    viz(this, Api) {
    deadbeef = Api;
    DBApi_main = this;
    DBApi_deadbeef = Api;

    // TODO: bad hack to keep shuffle/repeat actions in sync with PlaybackControl properties
    const QStringList ac_shuffle = {"q_shuffle_off", "q_shuffle_tracks", "q_shuffle_albums", "q_shuffle_random"};
    for (const QString &ac_str : ac_shuffle) {
        connect(&playback, &PlaybackControl::shuffleChanged, actions.getAction(ac_str), &DBAction::actionPropertiesChanged);
    }
    const QStringList ac_repeat = {"q_repeat_all", "q_repeat_one", "q_repeat_off"};
    for (const QString &ac_str : ac_repeat) {
        connect(&playback, &PlaybackControl::repeatChanged, actions.getAction(ac_str), &DBAction::actionPropertiesChanged);
    }
    //TrackRefc::override_ref();
    //playlist.setProperty("_dbapi_cover_cache", QVariant::fromValue(&cover));
    insertDefaultActions(this);
}

DBApi::~DBApi() {
    //freeDefaultActions(this);
}

int DBApi::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    playback.pluginMessage(id,ctx,p1,p2);
    playlist.pluginMessage(id,ctx,p1,p2);
    //cover.pluginMessage(id,ctx,p1,p2);
    return 0;
}


//void (*item_ref_orig)(DB_playItem_t*);
//void (*item_unref_orig)(DB_playItem_t*);
//QHash<DB_playItem_t *, int> refc_counter;



//void TrackRefc::override_ref() {
//    item_ref_orig = DBApi_deadbeef->pl_item_ref;
//    DBApi_deadbeef->pl_item_ref = TrackRefc::item_ref;
//    item_unref_orig = DBApi_deadbeef->pl_item_unref;
//    DBApi_deadbeef->pl_item_unref = TrackRefc::item_unref;
//}

//void TrackRefc::item_ref(DB_playItem_t *it) {
//    if (refc_counter.contains(it)) {
//        refc_counter.insert(it, refc_counter.value(it) + 1);
//        qDebug() << it << refc_counter.value(it);
//    }
//    else {
//        refc_counter.insert(it, 1);
//    }
//    item_ref_orig(it);
//}

//void TrackRefc::item_unref(DB_playItem_t *it) {
//    if (refc_counter.contains(it)) {
//        refc_counter.insert(it, refc_counter.value(it) - 1);
//        qDebug() << it << refc_counter.value(it);
//    }
//    else {
//        //qDebug() << "???";
//    }
//    item_unref_orig(it);
//}
