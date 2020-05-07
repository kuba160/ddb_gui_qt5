#include "CoverArtWidget.h"

#include <QStyle>
#include <QPixmap>

#include "QtGui.h"
#include "CoverArtCache.h"
#include <include/callbacks.h>
#include <QEvent>

#include "PluginLoader.h"
#include "MainWindow.h"

DB_plugin_t qtCoverart;

CoverArtWidget::CoverArtWidget(QWidget *parent):
        QDockWidget(parent),
        label(this),
        updateCoverAction(tr("Update cover"), &label) {
    setObjectName("CoverArt Widget");
    setWidget(&label);



    label.setContextMenuPolicy(Qt::ActionsContextMenu);
    label.addAction(&updateCoverAction);
    updateCoverAction.setIcon(getStockIcon(&label, "view-refresh", QStyle::SP_MediaPlay));
    //connect(w->Api(), SIGNAL(trackChanged(DB_playItem_t *, DB_playItem_t *)), w->Api(), SLOT(trackChanged(DB_playItem_t *, DB_playItem_t *)));
    connect(CoverArtCache::Instance(this), SIGNAL(coverIsReady(const QImage &)), SLOT(setCover(const QImage &)));
    connect(&updateCoverAction, SIGNAL(triggered(bool)), SLOT(reloadCover()));
    CACHE->getDefaultCoverArt();

    //qtCoverart.plugin.id = "coverart_qt";
    //qtCoverart.widget = this;
}

CoverArtWidget::~CoverArtWidget() {
    CACHE->Destroy();
}

void CoverArtWidget::trackChanged(DB_playItem_t *, DB_playItem_t *to) {
    if (isVisible())
        updateCover(to);
}

void CoverArtWidget::setCover(const QImage &aCover) {
    label.setPixmap(QPixmap::fromImage(aCover));
    setMaximumWidth(aCover.width() + 5);
    setMaximumHeight(aCover.height() + 25);
}

void CoverArtWidget::reloadCover() {
    DB_playItem_t *track = DBAPI->streamer_get_playing_track();
    if (!track)
        return;
    const char *album = DBAPI->pl_find_meta(track, "album");
    const char *artist = DBAPI->pl_find_meta(track, "artist");
    if (!album || !*album) {
        album = DBAPI->pl_find_meta(track, "title");
    }
    CACHE->removeCoverArt(artist, album);
    CACHE->getCoverArt(DBAPI->pl_find_meta(track, ":URI"), artist, album);
    if (track)
        DBAPI->pl_item_unref(track);
}

void CoverArtWidget::updateCover(DB_playItem_t *track) {
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

    CACHE->getCoverArt(DBAPI->pl_find_meta(track, ":URI"), artist, album);

    if (track)
        DBAPI->pl_item_unref(track);
}

void CoverArtWidget::closeEvent(QCloseEvent *event) {
    emit onCloseEvent();
    QDockWidget::closeEvent(event);
}

void CoverArtWidget::showEvent(QShowEvent *event) {
    updateCover();
    QDockWidget::showEvent(event);
}
