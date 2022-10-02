#include "StatusBar.h"

#define UPDATE_MSEC 100
#define DBAPI (this->api)

StatusBar::StatusBar(QWidget *parent, DBApi *Api) : QStatusBar(parent) {
    api = Api;
    label = new QLabel(this);
    addWidget(label, 1);
    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    timer.start(UPDATE_MSEC);

    QString script =  "$if2($upper(%codec%),-) |[ %playback_bitrate% kbps |][ %samplerate%Hz |][ %:BPS% bit |][ %channels% |] %playback_time% / %length% | ";
    track_script =  DBAPI->deadbeef->tf_compile (script.toUtf8());

    connect(api, SIGNAL(playbackPaused()), this, SLOT(update()));
    connect(api, SIGNAL(playbackUnPaused()), this, SLOT(update()));
    connect(api, SIGNAL(playbackStarted()), this, SLOT(update()));
    connect(api, SIGNAL(playbackStopped()), this, SLOT(update()));
    connect(api, SIGNAL(playlistChanged()), this, SLOT(update()));

    // macos size fix
    setAttribute(Qt::WA_MacSmallSize);
    label->setAttribute(Qt::WA_MacMiniSize);
    update();
}

StatusBar::~StatusBar() {
    DBAPI->deadbeef->tf_free(track_script);
    delete label;
}

QObject* StatusBar::constructor(QWidget *parent, DBApi *Api) {
    if (!parent) {
        QObject *info = new QObject(nullptr);
        info->setProperty("friendlyName", info->tr("Status Bar"));
        info->setProperty("internalName", "statusBar");
        info->setProperty("widgetType", "statusbar");
        info->setProperty("widgetStyle", "Qt Widgets");
        return info;
    }
    return new StatusBar(parent, Api);
}

// Taken and modified from DeaDBeef:plugins/gtkui/gtkui.c
void StatusBar::format_timestr (char *buf, int sz, float time) {
    time = qRound(time);
    int daystotal = (int)time / (3600*24);
    int hourtotal = ((int)time / 3600) % 24;
    int mintotal = ((int)time/60) % 60;
    int sectotal = ((int)time) % 60;

    if (daystotal == 0) {
        snprintf (buf, sz, "%d:%02d:%02d", hourtotal, mintotal, sectotal);
    }
    else if (daystotal == 1) {
        snprintf (buf, sz, tr("1 day %d:%02d:%02d").toUtf8(), hourtotal, mintotal, sectotal);
    }
    else {
        snprintf (buf, sz, tr("%d days %d:%02d:%02d").toUtf8(), daystotal, hourtotal, mintotal, sectotal);
    }
}

void StatusBar::update() {
    DB_playItem_t* curr_track = DBAPI->deadbeef->streamer_get_playing_track ();
    ddb_playlist_t *plt_curr = DBAPI->deadbeef->plt_get_curr ();
    ddb_tf_context_t context;
    context._size = sizeof(ddb_tf_context_t);
    context.flags = 0;
    context.it = curr_track;
    context.plt = plt_curr;
    context.idx = 0;
    context.id = 0;
    context.iter = PL_MAIN;
    context.update = 0;
    context.dimmed = 0;

    char buffer[512];
    buffer[0] = 0;

    int ret = DBAPI->deadbeef->tf_eval (&context, track_script, buffer, 512);

    QString str;

    switch (DBAPI->deadbeef->get_output()->state()) {
    case DDB_PLAYBACK_STATE_PAUSED:
        str.append(tr("Paused"));
        str.append(" | ");
        break;
    case DDB_PLAYBACK_STATE_STOPPED:
        str.append(tr("Stopped"));
        str.append(" | ");
    default:
        break;
    }
    if (curr_track) {
        str.append(buffer);
    }

    int it_count = DBAPI->deadbeef->plt_get_item_count (plt_curr, PL_MAIN);
    char total_playtime[64];
    format_timestr(total_playtime,64, DBAPI->deadbeef->plt_get_totaltime(plt_curr));
    str.append(QString("%1 %2 | %3 %4") .arg(it_count)
                                        .arg(tr("Tracks").toLower())
                                        .arg(total_playtime)
                                        .arg(tr("total playtime")));


    label->setText(str);

    if (curr_track) {
        DBAPI->deadbeef->pl_item_unref(curr_track);
    }
    if (plt_curr) {
        DBAPI->deadbeef->plt_unref(plt_curr);
    }

    timer.start(UPDATE_MSEC);
}
