#include "Playlist.h"
#include "../PlaylistView.h"

#define DBAPI Api->deadbeef

QObject * Playlist::constructor(QWidget *parent, DBApi *Api) {
    if (!parent) {
        QObject *info = new QObject(nullptr);
        info->setProperty("friendlyName", info->tr("Playlist"));
        info->setProperty("internalName", "playlist");
        info->setProperty("widgetType", "main");
        info->setProperty("widgetStyle", "Qt Widgets");
        return info;
    }

    PlaylistView *widget = new PlaylistView(parent, Api, parent->property("internalName").toString());

    widget->setModel(Api->playlist.getCurrentPlaylist());
    connect(widget,&QAbstractItemView::doubleClicked, widget, [widget,Api](const QModelIndex &idx) {Api->playback.play(idx.row());});
    connect(&Api->playlist, &PlaylistManager::currentPlaylistChanged, widget, [widget,Api](void) {
        int cursor = DBAPI->conf_get_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8(), -1);
        if (cursor != -1) {
            widget->setCurrentIndex(widget->model()->index(cursor, 0));
        }
    });
    return widget;
}

