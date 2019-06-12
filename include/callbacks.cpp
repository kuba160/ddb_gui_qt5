#include "callbacks.h"

#include <QWidget>
#include <QStyle>

#include "QtGui.h"
#include "qticonloader.h"

#ifdef ARTWORK_ENABLED
#include <plugins/CoverArt/CoverArtWrapper.h>
#endif

#ifdef ARTWORK_ENABLED
void cover_avail_callback(const char *fname, const char *artist, const char *album, void *user_data) {
    if (!fname && !artist && !album) {
        return;
    }
    char *image_fname = COVERART->get_album_art(fname, artist, album, -1, CALLBACK_COVER(&cover_avail_callback), NULL);
    if (image_fname) {
        CoverArtWrapper::Instance()->openAndScaleCover(image_fname);
    }
}

QImage *scale(const char *fname) {
    QImage *pm = new QImage(QString::fromUtf8(fname));
    if (pm->isNull()) {
        qDebug() << "Unsupported image format";
        delete pm;
        pm = new QImage(QString::fromUtf8(COVERART->get_default_cover()));
    }
    QImage *scaledImage = new QImage(pm->scaled(CoverArtWrapper::Instance()->defaultWidth, CoverArtWrapper::Instance()->defaultWidth, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    delete pm;
    return scaledImage;
}

#endif

void conf_get_str(const char *key, char *value, int len, const char *def) {
    DBAPI->conf_get_str(key, def, value, len);
}

QIcon getStockIcon(QWidget *widget, const QString &freedesktop_name, int fallback) {
    QIcon fallbackIcon;
    if (fallback > 0)
        fallbackIcon = widget->style()->standardIcon(QStyle::StandardPixmap(fallback), 0, widget);
    return QtIconLoader::icon(freedesktop_name, fallbackIcon);
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
