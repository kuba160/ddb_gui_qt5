#include "DBApi.h"

#include <QDebug>
#include <QWidget>
#include "QtGui.h"
#include "CoverArtCache.h"
#include "QtGuiSettings.h"
#include "ActionManager.h"
#include "DeadbeefTranslator.h"
#undef _
#undef DBAPI
#define DBAPI this->deadbeef

#define CAC (COVERARTCACHE_P(coverart_cache))
#define CSET (static_cast<QtGuiSettings *>(qt_settings))
#define AM (static_cast<ActionManager *>(action_manager))


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
            DBAPI->plt_get_title(DBAPI->plt_get_for_idx(i), title, sizeof(title));
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
}

DBApi::~DBApi() {
    clearClipboard();
    plugin.plugin.message = nullptr;
    delete CAC;
    delete CSET;
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

float DBApi::getPosition() {
    return DBAPI->playback_get_pos();
}

int DBApi::getCurrentPlaylist() {
    return DBAPI->plt_get_curr_idx();
}

QStringList DBApi::getPlaylists() {
    QStringList l;
    int count = DBAPI->plt_get_count();
    for (int i = 0; i < count; i++) {
        char buf[512];
        ddb_playlist_t *plt = DBAPI->plt_get_for_idx(i);
        if (plt) {
            DBAPI->plt_get_title(plt, buf, 512);
            l.append(QString(buf));
        }
        DBAPI->plt_unref(plt);
    }
    return l;
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

void DBApi::playTrackByIndex(uint32_t index) {
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
       emit playlistNamesChanged();
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
    emit playlistNamesChanged();
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
        emit playlistNamesChanged();
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
            emit playlistNamesChanged();
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
