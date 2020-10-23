#include "CoverArt.h"

#include <QStyle>
#include <QPixmap>

#include "QtGui.h"
#include "CoverArtCache.h"
#include <include/callbacks.h>
#include <QEvent>

#include "PluginLoader.h"
#include "MainWindow.h"

DB_plugin_t qtCoverart;

CoverArt::CoverArt(QWidget *parent, DBApi *api): QWidget(parent), DBWidget (parent, api) {

    setObjectName("CoverArt Widget");
    cover_display = new QLabel (this);
    m = new QMargins(10,10,10,10);

    connect (&api->coverart_cache, SIGNAL(coverLoaded(uint32_t, QImage*)), this, SLOT(onCoverLoaded(uint32_t, QImage*)));
    connect (&api->coverart_cache, SIGNAL(currCoverChanged(uint32_t)), this, SLOT(onCurrCoverChanged(uint32_t)));

    cover_display->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored);

    layout = new QHBoxLayout();
    layout->addWidget(cover_display);
    cover_display->setVisible(true);

    cover_image = api->coverart_cache.getDefaultCoverArt();
    cover_display->setPixmap(QPixmap::fromImage(cover_image->scaled(this->size().shrunkBy(*m),Qt::KeepAspectRatio,Qt::SmoothTransformation)));

    setLayout(layout);
//    cover_display->setPixmap(QPixmap::fromImage(image));
/*
 *     void coverLoaded(uint32_t request, QImage *image);

            //
            void currCoverChanged (uint32_t request);
    label.setContextMenuPolicy(Qt::ActionsContextMenu);
    label.addAction(&updateCoverAction);
    updateCoverAction.setIcon(getStockIcon(&label, "view-refresh", QStyle::SP_MediaPlay));
    //connect(w->Api(), SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)), w->Api(), SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    connect(CoverArtCache::Instance(this), SIGNAL(coverIsReady(const QImage &)), SLOT(setCover(const QImage &)));
    connect(&updateCoverAction, SIGNAL(triggered(bool)), SLOT(reloadCover()));
    CACHE->getDefaultCoverArt();
*/

    //qtCoverart.plugin.id = "coverart_qt";
    //qtCoverart.widget = this;
}

CoverArt::~CoverArt() {
    delete m;
    m = nullptr;
}

QWidget *CoverArt::constructor(QWidget *parent, DBApi *Api) {
    DB_plugin_t *artwork = Api->deadbeef->plug_get_for_id("artwork");
    if (!artwork) {
        return new QLabel(QString("Artwork plugin not available"));
    }
    return new CoverArt(parent, Api);
}

void CoverArt::onCurrCoverChanged(uint32_t handle) {
    // watch for cover with this handle
    handle_watch = handle;
}

void CoverArt::onCoverLoaded(u_int32_t handle, QImage *image) {
    if (handle == handle_watch) {
        // todo free previous etc etc.
        // check if cover is null
        cover_image = image;
        cover_display->setPixmap(QPixmap::fromImage(cover_image->scaled(this->size().shrunkBy(*m),Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        handle = 0;
    }
}

void CoverArt::resizeEvent(QResizeEvent *event) {
    //QImage *old_image = cover_image;
    if (cover_image) {

        cover_display->setPixmap(QPixmap::fromImage(cover_image->scaled(event->size().shrunkBy(*m),Qt::KeepAspectRatio,Qt::SmoothTransformation)));
    }
}
/*
void CoverArt::reloadCover() {
    DB_playItem_t *track = DBAPI->streamer_get_playing_track();
    if (!track)
        return;
    const char *album = DBAPI->pl_find_meta(track, "album");
    const char *artist = DBAPI->pl_find_meta(track, "artist");
    if (!album || !*album) {
        album = DBAPI->pl_find_meta(track, "title");
    }
    api->coverart_cache.removeCoverArt(album);
    api->coverart_cache.loadCoverArt(DBAPI->pl_find_meta(track, ":URI"), artist, album);
    if (track)
        DBAPI->pl_item_unref(track);
}

void CoverArt::updateCover(DB_playItem_t *track) {
    if (!track)
        track = DBAPI->streamer_get_playing_track();
    else
        DBAPI->pl_item_ref(track);

    if (!track)
        return;

    const char *album = DBAPI->pl_find_meta(track, "album");
    const char *artist = DBAPI->pl_find_meta(track, "artist");
    if (!album || !*album) {
        album = DBAPI->pl_find_meta(track, "title");
    }

    api->coverart_cache.loadCoverArt(DBAPI->pl_find_meta(track, ":URI"), artist, album);

    if (track)
        DBAPI->pl_item_unref(track);
}
*/
