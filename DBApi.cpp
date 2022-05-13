#include "DBApi.h"

#include <QDebug>
#include <QWidget>
#include <QDialog>
#include <QtConcurrent>
#include "QtGui.h"
#include "CoverArtCache.h"
#include "QtGuiSettings.h"
#include "ActionManager.h"
#include "DeadbeefTranslator.h"
#include "PlaylistBrowserModel.h"
#include "PlaylistModel.h"
#include "PlayqueueModel.h"
#include "ScopeWrapper.h"

#undef _
#undef DBAPI
#define DBAPI this->deadbeef

#define CAC (COVERARTCACHE_P(coverart_cache))
#define CSET (static_cast<QtGuiSettings *>(qt_settings))
#define AM (static_cast<ActionManager *>(action_manager))
#define PM (static_cast<PlaylistModel *>(cpl))


DBApi::DBApi(QObject *parent, DB_functions_t *Api) : QObject(parent) {
    this->deadbeef = Api;
    if (DBAPI->conf_get_int("resume.paused", 0)) {
        // will be paused
        internal_state = DDB_PLAYBACK_STATE_PAUSED;
    }
    else {
        // can be playing or stopped
        internal_state = DBAPI->get_output() ? DBAPI->get_output()->state() : DDB_PLAYBACK_STATE_STOPPED;
    }


    // playlists
    {
        int playlistCount = DBAPI->plt_get_count();
        char title[100];
        for (int i = 0; i < playlistCount; i++) {
            DBAPI->pl_lock();
            ddb_playlist_t *plt = DBAPI->plt_get_for_idx(i);
            DBAPI->plt_get_title(plt, title, sizeof(title));
            DBAPI->plt_unref(plt);
            DBAPI->pl_unlock();
            playlistNames.push_back(QString::fromUtf8(title));
            strcpy(title, "");
        }
    }

    // shuffle/repeat
    currShuffle = DBAPI->streamer_get_shuffle();
    currRepeat = DBAPI->streamer_get_repeat();

    // CoverArt Cache
    coverart_cache = new CoverArtCache(this, Api);

    // Settings
    qt_settings = new QtGuiSettings(this);
    settings = static_cast<QtGuiSettings *>(qt_settings);

    // Action Manager
    action_manager = new ActionManager(this,this);

    // clipboard
    clipboard = QGuiApplication::clipboard();

    // volume
    m_volume = DBAPI->volume_get_db();

    // Playlist browser model
    pbm = new PlaylistBrowserModel(nullptr, this);

    // Current playlist model
    cpl = new PlaylistModel(nullptr, this);
    ddb_playlist_t *plt_new = DBAPI->plt_get_curr();
    PM->setPlaylist(plt_new);
    DBAPI->plt_unref(plt_new);
    connect(this, SIGNAL(currentPlaylistChanged()), this, SLOT(onCurrentPlaylistChanged()));
    PM->setDefaultHeaderSettings();

    // Queue
    qm = new PlayqueueModel(nullptr, this);

    // Current playing model
    cpm = new CurrentPlayItemModel(nullptr, this);

}

DBApi::~DBApi() {
    clearClipboard();
    plugin.plugin.message = nullptr;
    delete CAC;
    delete CSET;
    delete pbm;
    delete cpl;
    delete qm;
    delete cpm;
}

const char * DBApi::_(const char *str) {
    return dbtr->translate(nullptr, str).toUtf8().constData();
}

int DBApi::pluginMessage(uint32_t id, uintptr_t ctx, uint32_t p1, uint32_t p2) {
    Q_UNUSED(p2);

    ddb_event_trackchange_t *ev;
    ddb_playback_state_t state;
    switch (id) {
        case DB_EV_SONGCHANGED:
            emit stateStoppedChanged();
            emit statePlayingChanged();
            emit statePausedChanged();
            ev = (ddb_event_trackchange_t *)ctx;
            if (ev->to) {
                internal_state = DDB_PLAYBACK_STATE_PLAYING;
            }
            else {
                internal_state = DDB_PLAYBACK_STATE_STOPPED;
            }
            emit trackChanged(ev->from, ev->to);
            emit trackChanged();
            emit queueChanged();
            emit playingLengthChanged();
            break;
        case DB_EV_PAUSED:
            emit stateStoppedChanged();
            emit statePlayingChanged();
            emit statePausedChanged();
            state = p1 ? DDB_PLAYBACK_STATE_PAUSED : DDB_PLAYBACK_STATE_PLAYING;
            if (internal_state != state) {
                internal_state = state;
                if (p1) {
                    emit playbackPaused();
                }
                else {
                    emit playbackUnPaused();
                    emit playbackStarted();
                }
            }
            break;
        case DB_EV_ACTIVATED:
            emit deadbeefActivated();
            break;
        case DB_EV_VOLUMECHANGED:
            emit volumeChanged();
            break;
        case DB_EV_PLAY_NUM:
        case DB_EV_PLAY_CURRENT:
        case DB_EV_PLAY_RANDOM:
        case DB_EV_SONGSTARTED:
            emit stateStoppedChanged();
            emit statePlayingChanged();
            emit statePausedChanged();
            emit playbackStarted();
            emit playingLengthChanged();
            break;
        case DB_EV_STOP:
            emit stateStoppedChanged();
            emit statePlayingChanged();
            emit statePausedChanged();
            internal_state = DDB_PLAYBACK_STATE_STOPPED;
            emit playbackStopped();
            emit queueChanged();
            break;
        case DB_EV_DSPCHAINCHANGED:
            // update eq state
            // TODO detect change
            emit eqAvailableChanged();
            emit eqEnabledChanged();
            break;
        case DB_EV_PLAYLISTCHANGED:
        case DB_EV_TRACKINFOCHANGED:
            // detect queue
            if (p1 == DDB_PLAYLIST_CHANGE_PLAYQUEUE) {
                emit queueChanged();
            }
            emit stateStoppedChanged();
            emit statePlayingChanged();
            emit statePausedChanged();
            // output state?
            ddb_playback_state_t output_state = internal_state; //DBAPI->get_output()->state();
            if (internal_state != output_state) {
                internal_state = output_state;
                if (internal_state == DDB_PLAYBACK_STATE_PAUSED) {
                    emit playbackPaused();
                }
                else if (internal_state == DDB_PLAYBACK_STATE_STOPPED) {
                    emit playbackStopped();
                }
                else {
                    // DDB_PLAYBACK_STATE_PLAYING
                    emit playbackUnPaused();
                    emit playbackStarted();
                }
            }
            break;

    }
    return 0;
}

QString const& DBApi::playlistNameByIdx(int idx) {
    char buf[512];
    static QString str;
    ddb_playlist_t *plt = DBAPI->plt_get_for_idx(idx);
    if (plt) {
        DBAPI->plt_get_title(plt, buf, 512);
        DBAPI->plt_unref(plt);
        return str = buf;
    }
    // old method
    static QString empty;
    if (idx >= playlistNames.size()) {
        return empty;
    }
    return playlistNames.at(idx);
}

unsigned long DBApi::getPlaylistCount() {
    return playlistNames.size();
}

bool DBApi::isPaused() {
    return DBAPI->get_output()->state() == DDB_PLAYBACK_STATE_PAUSED;
    //return (internal_state == DDB_PLAYBACK_STATE_PAUSED) ? true : false;
}

float DBApi::getVolume() {
    return DBAPI->volume_get_db();
}

ddb_playback_state_t DBApi::getOutputState() {
    return internal_state;
    if (DBAPI->get_output()) {
        return DBAPI->get_output()->state();
    }
    return DDB_PLAYBACK_STATE_STOPPED;
}

ddb_playback_state_t DBApi::getInternalState() {
    return internal_state;
}

void DBApi::playItemContextMenu(QWidget *w, QPoint p) {
    AM->playItemContextMenu(w,p);
}

void DBApi::playlistContextMenu(QWidget *w, QPoint p, int plt) {
    AM->playlistContextMenu(w,p, plt);
}

QMenuBar * DBApi::getMainMenuBar() {
    return AM->mainMenuBar;
}

QMenu * DBApi::getMenu(const char *menu) {
    QMenuBar *mb = AM->mainMenuBar;
    return mb->findChild<QMenu *>(_(menu));
}

void DBApi::clearClipboard() {
    if (clipboard->mimeData()->hasFormat("deadbeef/playitems")) {
        playItemList l = mime_playItems(clipboard->mimeData());
        foreach(DB_playItem_t *it, l) {
            DBAPI->pl_item_unref(it);
        }
    }
}

QMimeData *DBApi::mime_playItems(QList<DB_playItem_t *> playItems) {
    QMimeData *md = new QMimeData();
    QByteArray ba;
    QDataStream ds(&ba,QIODevice::WriteOnly);
    for(int i = 0; i < playItems.length() ; i++) {
        // original
        auto ptr = reinterpret_cast<quintptr>(playItems.at(i));
        ds << ptr;
    }
    md->setData("deadbeef/playitems", ba);
    return md;
}

QList<DB_playItem_t *> DBApi::mime_playItems(const QMimeData *playItems) {
    QList<DB_playItem_t *> list;
    if (playItems->hasFormat("deadbeef/playitems")) {
        QByteArray ba = playItems->data("deadbeef/playitems");
        QDataStream ds(ba);
        while (!ds.atEnd()) {
            quintptr p;
            ds >> p;
            list.append(reinterpret_cast<DB_playItem_t *>(p));
        }
    }
    return list;
}

QList<DB_playItem_t *> DBApi::mime_playItemsCopy(const QMimeData *playItems) {
    QList<DB_playItem_t *> list;
    if (playItems->hasFormat("deadbeef/playitems")) {
        QByteArray ba = playItems->data("deadbeef/playitems");
        QDataStream ds(ba);
        while (!ds.atEnd()) {
            quintptr p;
            ds >> p;
            DB_playItem_t *it = reinterpret_cast<DB_playItem_t *>(p);
            DB_playItem_t *it_new = DBAPI->pl_item_alloc();
            DBAPI->pl_item_copy(it_new,it);
            list.append(it_new);
        }
    }
    return list;
}

QMimeData *DBApi::mime_playItemsCopy(QList<DB_playItem_t *> playItems) {
    QMimeData *md = new QMimeData();
    QByteArray ba;
    QDataStream ds(&ba,QIODevice::WriteOnly);
    for(int i = 0; i < playItems.length() ; i++) {
        // original
        DB_playItem_t *it = DBAPI->pl_item_alloc();
        DBAPI->pl_item_copy(it,playItems[i]);
        auto ptr = reinterpret_cast<quintptr>(it);
        ds << ptr;
    }
    md->setData("deadbeef/playitems", ba);
    return md;
}

void DBApi::confSetValue(const QString &plugname, const QString &key, const QVariant &value) {
    settings->setValue(plugname,key,value);
}

QVariant DBApi::confGetValue(const QString &plugname, const QString &key, const QVariant &defaultValue) {
    return settings->getValue(plugname,key,defaultValue);
}

void DBApi::addTracksByUrl(const QUrl &url, int position) {
    int pabort = 0;
    DB_playItem_t *track = (position > -1) ? DBAPI->pl_get_for_idx(position) : nullptr;
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    DB_playItem_t *it = DBAPI->plt_insert_dir(plt, track, url.toString().toUtf8().data(), &pabort, nullptr, nullptr);
    if (!it) {
        it = DBAPI->plt_insert_file(plt, track, url.toString().toUtf8().data(), &pabort, nullptr, nullptr);
    }
    if (it) {
        emit playlistContentChanged(plt);
    }
    if (track) {
        DBAPI->pl_item_unref(track);
    }
    DBAPI->plt_unref(plt);
}

struct ApiBtn {
    DB_functions_t *func;
    QPushButton *pb;
};

void DBApi::addTracks(QStringList list, bool directories) {
    add_abort = 0;
    int track_last = DBAPI->pl_getcount(PL_MAIN) - 1;
    DB_playItem_t *track = track_last > 0 ? DBAPI->pl_get_for_idx(track_last) : nullptr;
    ddb_playlist_t *plt = DBAPI->plt_get_curr();

    // Dialog
    QDialog *d = new QDialog(w, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    d->setWindowTitle(tr("Adding files..."));
    d->setAttribute(Qt::WA_DeleteOnClose);
    if (d->layout()) {
        //delete d->layout(); // ?
    }
    QVBoxLayout *lay = new QVBoxLayout;
    QLineEdit *le = new QLineEdit;
    le->setReadOnly(true);
    //lay.insertWidget(-1, &le);
    lay->addWidget(le);
    QPushButton *pb = new QPushButton;
    pb->setText(tr("Cancel"));
    lay->addWidget(pb);
    //lay.insertWidget(-1,&pb,0,Qt::AlignRight);
    connect (pb, SIGNAL(clicked()), this, SLOT(onBtnCancelAdd()));
    d->setLayout(lay);
    le->setVisible(true);
    pb->setVisible(true);

    le->setText("aaaa");

    d->open();


    struct ApiBtn ab = {api->deadbeef, pb};

    QFuture<DB_playItem_t *> f;
    DB_playItem_t *it = nullptr;
    if (directories) {
        foreach (QString s, list) {
            f = QtConcurrent::run(add_files,s, &add_abort, &ab);
            //it = DBAPI->plt_insert_dir(plt, track, s.toUtf8().data(), &add_abort, add_file_callback, &ab);
        }
    }
    else {
        foreach (QString s, list) {
            it = DBAPI->plt_insert_file(plt, track, s.toUtf8().data(), &add_abort, add_file_callback, &ab);
        }
    }

    f.waitForFinished();

    if (it) {
        emit playlistContentChanged(plt);
    }
    else {
        // failure
    }
    if (track) {
        DBAPI->pl_item_unref(track);
    }
    DBAPI->plt_unref(plt);
    d->close();
}

DB_playItem_t* DBApi::add_files(QStringList l, bool directories, DBApi *api, int *pabort) {
    int add_abort = 0;
    int track_last = api->deadbeef->pl_getcount(PL_MAIN) - 1;
    DB_playItem_t *track = track_last > 0 ? api->deadbeef->pl_get_for_idx(track_last) : nullptr;
    ddb_playlist_t *plt = api->deadbeef->plt_get_curr();

    // Dialog
    QDialog *d = new QDialog(w, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    d->setWindowTitle(tr("Adding files..."));
    d->setAttribute(Qt::WA_DeleteOnClose);
    if (d->layout()) {
        //delete d->layout(); // ?
    }
    QVBoxLayout *lay = new QVBoxLayout;
    QLineEdit *le = new QLineEdit;
    le->setReadOnly(true);
    //lay.insertWidget(-1, &le);
    lay->addWidget(le);
    QPushButton *pb = new QPushButton;
    pb->setText(tr("Cancel"));
    lay->addWidget(pb);
    //lay.insertWidget(-1,&pb,0,Qt::AlignRight);
    connect (pb, SIGNAL(clicked()), this, SLOT(onBtnCancelAdd()));
    d->setLayout(lay);
    le->setVisible(true);
    pb->setVisible(true);

    le->setText("aaaa");

    d->open();


    struct ApiBtn ab = {api->deadbeef, pb};

    QFuture<DB_playItem_t *> f;
    DB_playItem_t *it = nullptr;
    if (directories) {
        foreach (QString s, list) {
            f = QtConcurrent::run(add_file,s, &add_abort, &ab);
            //it = DBAPI->plt_insert_dir(plt, track, s.toUtf8().data(), &add_abort, add_file_callback, &ab);
        }
    }
    else {
        foreach (QString s, list) {
            it = DBAPI->plt_insert_file(plt, track, s.toUtf8().data(), &add_abort, add_file_callback, &ab);
        }
    }

    f.waitForFinished();

    if (it) {
        emit playlistContentChanged(plt);
    }
    else {
        // failure
    }
    if (track) {
        DBAPI->pl_item_unref(track);
    }
    DBAPI->plt_unref(plt);
    d->close();



    struct ApiBtn *ab = static_cast<struct ApiBtn*>(user_data);
    int track_last = ab->func->pl_getcount(PL_MAIN) - 1;
    DB_playItem_t *track = track_last > 0 ? ab->func->pl_get_for_idx(track_last) : nullptr;
    ddb_playlist_t *plt = ab->func->plt_get_curr();
    DB_playItem_t *it = ab->func->plt_insert_dir(plt, track, s.toUtf8().data(), pabort, add_file_callback, &ab);
    return it;
}

int DBApi::add_file_callback (DB_playItem_t *it, void *user_data) {
    if (user_data) {
        struct ApiBtn *ab = static_cast<struct ApiBtn*>(user_data);
        const char *f = ab->func->pl_find_meta_raw (it, ":URI");
        if (f) {
            qDebug() << f;
            ab->pb->setText(f);
        }
    }
    return 0;
}

void DBApi::onBtnCancelAdd() {
    add_abort = 1;
}

float DBApi::getPosition() {
    return DBAPI->playback_get_pos();
}

int DBApi::getCurrentPlaylist() {
    return DBAPI->plt_get_curr_idx();
}

// isPaused goes here...

bool DBApi::isPlaying() {
    return DBAPI->get_output()->state() == DDB_PLAYBACK_STATE_PLAYING;
}

bool DBApi::isStopped() {
    return DBAPI->get_output()->state() == DDB_PLAYBACK_STATE_STOPPED;
}

float DBApi::getPlayingLength() {
    DB_playItem_t *it = DBAPI->streamer_get_playing_track();
    float len = 0.0;
    if (it) {
        len = DBAPI->pl_get_item_duration(it);
        DBAPI->pl_item_unref(it);
    }
    return len;
}

// slots

void DBApi::setVolume(float value) {
    if (m_volume != value) {
        DBAPI->volume_set_db(value);
        m_volume = value;
        emit volumeChanged();
    }
}

void DBApi::setVolume(int value) {
    if (DBAPI->volume_get_db() != value) {
        DBAPI->volume_set_db(value);
        m_volume = value;
        emit volumeChanged();
    }
}

void DBApi::setPosition(float pos) {
    DBAPI->playback_set_pos(pos);
    emit positionChanged();
}

void DBApi::setCurrentPlaylist(int plt) {
    if (DBAPI->plt_get_curr_idx() != plt) {
        if (plt < DBAPI->plt_get_count() && plt >= 0) {
            changePlaylist(plt);
            emit currentPlaylistChanged();
        }
    }
}

void DBApi::setPaused(bool pause) {
    if (internal_state == DDB_PLAYBACK_STATE_PLAYING && pause) {
        togglePause();
        emit statePausedChanged();
        emit statePlayingChanged();
    }
    else if (internal_state == DDB_PLAYBACK_STATE_PAUSED && !pause) {
        togglePause();
        emit statePausedChanged();
        emit statePlayingChanged();
    }
}

void DBApi::setPlaying(bool playing) {
    if (internal_state != DDB_PLAYBACK_STATE_PLAYING && playing) {
        play();
        emit statePlayingChanged();
        emit stateStoppedChanged();
    }
    else if (internal_state == DDB_PLAYBACK_STATE_PLAYING && !playing) {
        stop();
        emit statePlayingChanged();
        emit stateStoppedChanged();
    }
}

void DBApi::setStopped(bool stopped) {
    if (internal_state != DDB_PLAYBACK_STATE_STOPPED && stopped) {
        stop();
        emit stateStoppedChanged();
        if (internal_state == DDB_PLAYBACK_STATE_PLAYING)
            emit statePlayingChanged();
        else
            emit statePausedChanged();
    }
    else if (internal_state == DDB_PLAYBACK_STATE_STOPPED && !stopped) {
        play();
    }
}

ddb_dsp_context_t * DBApi::get_supereq() {
    ddb_dsp_context_t *eq_ctx = DBAPI->streamer_get_dsp_chain();
    ddb_dsp_context_t *dsp = eq_ctx;
    while (dsp) {
        if (strcmp(dsp->plugin->plugin.id, "supereq") == 0) {
            return dsp;
        }
        dsp = dsp->next;
    }
    return nullptr;
}

bool DBApi::isEqAvailable() {
    return get_supereq() ? true : false;
}

bool DBApi::isEqEnabled() {
    ddb_dsp_context_t *supereq = get_supereq();
    return supereq->enabled;
}

QList<float> DBApi::getEq() {
    QList<float> l;
    ddb_dsp_context_t *supereq = get_supereq();
    if (supereq) {
        char buf[255];
        int param_count = supereq->plugin->num_params();
        for (int i = 0; i < param_count; i++) {
            supereq->plugin->get_param(supereq, i, buf, 255);
            l.append(atof(buf));
        }
    }
    else {
        qDebug() << "getEq(): supereq unavailable!";
    }
    return l;
}

QAbstractListModel* DBApi::getPlaylistBrowserModel() {
    return pbm;
}

QAbstractItemModel* DBApi::getCurrentPlaylistModel() {
    return cpl;
}

QAbstractItemModel* DBApi::getQueueModel() {
    return qm;
}

QAbstractItemModel* DBApi::getCurrentPlayingModel() {
    return cpm;
}

QColor DBApi::getAccentColor() {
    QVariant c = SETTINGS->getValue("MainWindow","accent_color");
    if (c.isValid()) {
        return c.value<QColor>();
    }
    else {
        QColor c = QGuiApplication::palette().color(QPalette::Active, QPalette::Highlight);
        // TODO dirty hack to get color closer to accent color (on KDE) by lighting it up a bit
        // does not perform if color is light enough
        c.setHsv(c.hue(),c.saturation(),c.value() < 133 ? c.value()+64 : c.value());
        return c;
    }
}

void DBApi::setAccentColor(QColor c) {
    SETTINGS->setValue("MainWindow", "accent_color", c);
    emit accentColorChanged();
}

QObject* DBApi::scope_create(QObject *parent) {
    if (!scope_list.length()) {
         deadbeef->vis_waveform_listen(this,waveform_callback);
    }
    ScopeWrapper *wr = new ScopeWrapper(parent);
    connect(wr, SIGNAL(destroyed(QObject*)), this, SLOT(onScopeDestroyed(QObject*)));
    scope_list.append(wr);
    return wr;
}

void DBApi::onScopeDestroyed(QObject *obj) {
    if (scope_list.contains(obj)) {
        scope_list.removeAll(obj);
    }
    if (!scope_list.length()) {
        deadbeef->vis_waveform_unlisten(this);
    }
}

void DBApi::waveform_callback (void * ctx, const ddb_audio_data_t *data) {
    DBApi *api = static_cast<DBApi *>(ctx);

    for (int i = 0; i < api->scope_list.length(); i++) {
        ScopeWrapper *sw = static_cast<ScopeWrapper*>(api->scope_list.at(i));
        sw->process(data);
    }
}

void DBApi::setEqEnabled(bool enable) {
    ddb_dsp_context_t *supereq = get_supereq();
    if (supereq) {
        if (supereq->enabled != enable) {
            supereq->enabled = enable;
            DBAPI->streamer_dsp_refresh ();
            DBAPI->streamer_dsp_chain_save ();
            emit eqEnabledChanged();
        }
    }
    else {
        qDebug() << "setEqEnabled(): supereq unavailable!";
    }
}

void DBApi::setEq(QList<float> eq) {
    ddb_dsp_context_t *supereq = get_supereq();
    if (supereq) {
        QList<float> eq_curr = getEq();
        bool changed = false;
        int param_count = supereq->plugin->num_params();
        for (int i = 0; i < param_count; i++) {
            if (eq[i] != eq_curr[i]) {
                if (eq[i] > 20.0) {
                    eq[i] = 20.0;
                }
                else if (eq[i] < -20.0) {
                    eq[i] = -20.0;
                }
                char buf[255];
                snprintf(buf, 255, "%f", eq[i]);
                supereq->plugin->set_param(supereq, i, buf);
                changed = true;
            }
        }
        if (changed) {
            DBAPI->streamer_dsp_refresh();
            DBAPI->streamer_dsp_chain_save();
            emit eqChanged();
        }
    }
    else {
        qDebug() << "setEq(): supereq unavailable!";
    }
}

void DBApi::playTrackByIndex(quint32 index) {
    DBAPI->sendmessage(DB_EV_PLAY_NUM, 0, index, 0);
}

void DBApi::sendPlayMessage(uint32_t id) {
    DBAPI->sendmessage(id, 0, 0, 0);
}

void DBApi::togglePause() {
    DBAPI->sendmessage(DB_EV_TOGGLE_PAUSE, 0, 0, 0);
}

void DBApi::play() {
    DBAPI->sendmessage(DB_EV_PLAY_CURRENT, 0, 0, 0);
}

void DBApi::stop() {
    DBAPI->sendmessage(DB_EV_STOP, 0, 0, 0);
}

void DBApi::playNext() {
    DBAPI->sendmessage(DB_EV_NEXT, 0, 0, 0);
}

void DBApi::playPrev() {
    DBAPI->sendmessage(DB_EV_PREV, 0, 0, 0);
}

void DBApi::changePlaylist(int idx) {
    if (idx < playlistNames.size() && playlist_internal != idx) {
        playlist_internal = idx;
        DBAPI->plt_set_curr_idx(idx);
        DBAPI->conf_set_int("playlist.current", idx);
        emit playlistChanged(idx);
        emit playlistChanged();
        emit currentPlaylistChanged();
    }
}

void DBApi::movePlaylist(int plt, int before) {
    if (plt != before) {
       DBAPI->plt_move(plt, before);
       playlistNames.move(plt, before);
       if (plt == playlist_internal) {
           playlist_internal = before;
       }
       emit playlistMoved(plt, before);
    }
}

void DBApi::newPlaylist(QString name) {
    // TODO add new playlist location
    int count = 0;
    for (int i = 0; i < DBAPI->plt_get_count(); i++) {
        char buf[512];
        DBAPI->plt_get_title(DBAPI->plt_get_for_idx(i), buf, 512);
        QRegularExpression re(name + "( \\([1-9]+\\))?");
        if (re.match(buf).hasMatch()) {
            count++;
        }
    }
    if (count) {
        name.append(QString(" (%1)").arg(count));
    }
    DBAPI->plt_add (DBAPI->plt_get_count(), name.toUtf8());
    playlistNames.append(name);
    emit playlistCreated();
}

void DBApi::renamePlaylist(int plt, const QString *name) {
    if (plt < playlistNames.size()) {
        DBAPI->pl_lock ();
        ddb_playlist_t *plt_p = DBAPI->plt_get_for_idx(plt);
        DBAPI->plt_set_title (plt_p, name->toUtf8());
        DBAPI->plt_unref (plt_p);
        DBAPI->pl_unlock ();
        playlistNames.insert(plt, *name);
        playlistNames.removeAt(plt+1);
        emit playlistRenamed(plt);
    }
}

void DBApi::renamePlaylist(int plt) {
    if (plt < playlistNames.size()) {
        bool ok;
        QString newName = QInputDialog::getText(w, tr("Rename Playlist"), tr("Rename Playlist") + ":",
                                                QLineEdit::Normal, playlistNameByIdx(plt), &ok);
        if (ok && !newName.isEmpty()) {
            renamePlaylist(plt, &newName);
        }
    }
}

void DBApi::removePlaylist(int plt) {
    if (plt < playlistNames.size()) {
        // Dialog?
        QString question = tr("Do you really want to remove the playlist '%s'?");
        question.replace("%s", playlistNameByIdx(plt));
        QMessageBox confirmation(QMessageBox::Question,tr("Remove Playlist"),
                                 question, QMessageBox::Yes | QMessageBox::No, w);
        int ret = confirmation.exec();
        if (ret == QMessageBox::Yes) {
            if (plt == DBAPI->plt_get_curr_idx()) {
                changePlaylist(plt-1 > 0 ? plt-1 : 0);
            }
            DBAPI->plt_remove(plt);
            emit playlistRemoved(plt);
        }
    }
}

void DBApi::clearPlaylist(int plt) {
    if (plt < playlistNames.size()) {
        ddb_playlist_t *plt_p = DBAPI->plt_get_for_idx(plt);
        DBAPI->plt_clear(plt_p);
        emit playlistContentChanged(plt_p);
        DBAPI->plt_unref(plt_p);
    }
}

void DBApi::loadPlaylist(const QString &fname) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    if (plt) {
        DBAPI->plt_clear(plt);
        int abort = 0;
        DB_playItem_t *it = DBAPI->plt_load2(-1, plt, NULL, fname.toUtf8().constData(), &abort, NULL, NULL);
        if (it) {
            // success
            emit playlistContentChanged(plt);
        }
        DBAPI->plt_unref(plt);
    }
}

void DBApi::onCurrentPlaylistChanged() {
    ddb_playlist_t *plt_new = DBAPI->plt_get_curr();
    static_cast<PlaylistModel *>(cpl)->setPlaylist(plt_new);
    DBAPI->plt_unref(plt_new);
}

void DBApi::removeTracks(playItemList list) {
    if (list.count()) {
        ddb_playlist_t *plt = DBAPI->pl_get_playlist(list[0]);
        foreach(DB_playItem_t *it, list) {
            DBAPI->plt_remove_item(plt,it);
            DBAPI->pl_item_unref(it);
        }
        emit playlistContentChanged(plt);
        DBAPI->plt_unref(plt);
    }
}

void DBApi::setShuffle(ddb_shuffle_t i) {
    DBAPI->streamer_set_shuffle(i);
    currShuffle = i;
    emit shuffleChanged();
}

void DBApi::setRepeat(ddb_repeat_t i) {
    DBAPI->streamer_set_repeat(i);
    currRepeat = i;
    emit repeatChanged();
}

bool DBApi::isCoverArtPluginAvailable() {
    return CAC->backend ? true : false;
}

bool DBApi::isCoverArtCached(DB_playItem_t *it, QSize size) {
    return CAC->isCoverArtAvailable(it, size);
}

QFuture<QImage *> DBApi::requestCoverArt(DB_playItem_t *p, QSize size) {
    return CAC->requestCoverArt(p, size);
}

QImage * DBApi::getCoverArt(DB_playItem_t *it, QSize size) {
    return CAC->getCoverArt(it, size);
}

QImage * DBApi::getCoverArtDefault() {
    return CAC->getCoverArtDefault();
}

void DBApi::coverArt_ref(QImage *img) {
    CAC->cacheRef(img);
}

void DBApi::coverArt_unref(QImage *img) {
    CAC->cacheUnref(img);
}

void DBApi::coverArt_track_unref(DB_playItem_t *it) {
    CAC->cacheUnrefTrack(it);
}

QDataStream &operator<<(QDataStream &ds, const playItemList &pil) {
    qint32 i;
    for (i = 0; i < pil.length(); i++) {
        auto ptr= reinterpret_cast<quintptr>(pil[i]);
        ds << ptr;
    }
    return ds;
}
QDataStream& operator >> (QDataStream &ds, playItemList &pil) {
    pil.clear();
    while (!ds.atEnd()) {
        quintptr ptrval;
        ds >> ptrval;
        auto temp = reinterpret_cast<DB_playItem_t *>(ptrval);
        pil.append(temp);
    }
    return ds;
}
