#include "callbacks.h"

#include <QWidget>
#include <QStyle>

#include "QtGui.h"
//#include "qticonloader.h"
#undef DBAPI
#define DBAPI deadbeef_internal

QIcon getStockIcon(QWidget *widget, const QString &freedesktop_name, int fallback) {
    QIcon fallbackIcon;
    if (fallback > 0) {
        fallbackIcon = widget->style()->standardIcon(QStyle::StandardPixmap(fallback), 0, widget);
    }
    return fallbackIcon.isNull() ? QIcon::fromTheme(freedesktop_name, fallbackIcon) : fallbackIcon;
}

void loadPlaylist(const QString &fname) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    if (plt) {
        DBAPI->plt_clear(plt);
        int abort = 0;
        DB_playItem_t *it = DBAPI->plt_load(plt, NULL, fname.toUtf8().constData(), &abort, NULL, NULL);
        if (it) {
            DBAPI->pl_item_unref(it);
        }
        DBAPI->plt_unref(plt);
    }
}

void loadAudioCD() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    DBAPI->plt_add_file(plt, "all.cda", NULL, NULL);
    DBAPI->plt_unref(plt);
}
