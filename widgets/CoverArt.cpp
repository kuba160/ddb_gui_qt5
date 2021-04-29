#include "CoverArt.h"
#include "QtGui.h"

CoverArt::CoverArt(QWidget *parent, DBApi *api): QWidget(parent), DBWidget (parent, api) {
    cover_display.setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored);
    cover_display.setAlignment(Qt::AlignHCenter);
    setMinimumSize(64,64);
    m = QMargins(10,10,10,10);
    layout.addWidget(&cover_display);
    setLayout(&layout);
    cover_default = api->getCoverArtDefault();

    connect(&fw, SIGNAL(finished()), this, SLOT(refreshCover()));
    connect(api,SIGNAL(trackChanged(DB_playItem_t*,DB_playItem_t*)), this,SLOT(onTrackChanged(DB_playItem_t*,DB_playItem_t*)));
    connect(api, SIGNAL(playbackStopped()), this, SLOT(onPlaybackStopped()));

    cover_size = QSize(width() - m.left() - m.right(), height() - m.top() - m.bottom());

    // load cover if avail
    DB_playItem_t *it = DBAPI->streamer_get_playing_track();
    onTrackChanged(nullptr, it);
    if (it) {
        DBAPI->pl_item_unref(it);
    }
}

CoverArt::~CoverArt() {
    if (cover_default) {
        api->coverArt_unref(cover_default);
    }
    if (cover_image && cover_image != cover_default) {
        api->coverArt_unref(cover_image);
    }
}

QWidget *CoverArt::constructor(QWidget *parent, DBApi *Api) {
    if (!Api->isCoverArtPluginAvailable()) {
        return new QLabel(QString("Artwork plugin not available"));
    }
    return new CoverArt(parent, Api);
}

void CoverArt::onPlaybackStopped() {
    onTrackChanged(nullptr,nullptr);
}

void CoverArt::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    Q_UNUSED(from)
    if (!to) {
        if (cover_image && cover_image != cover_default) {
            api->coverArt_unref(cover_image);
        }
        cover_image = cover_default;
        refreshCover();
        return;
    }
    bool avail = api->isCoverArtCached(to);
    if (avail) {
        QImage *img = api->getCoverArt(to);
        if (cover_image != img) {
            if (cover_image && cover_image != cover_default) {
                api->coverArt_unref(cover_image);
            }
            cover_image = img;
            refreshCover();
        }
        else {
            // same image, just unref
            api->coverArt_unref(cover_image);
        }
    }
    else {
        // cache and watch
        fw.setFuture(api->requestCoverArt(to));
    }

}

void CoverArt::refreshCover() {
    if (fw.isFinished() &&  !fw.isCanceled()) {
        if (cover_image && cover_image != cover_default) {
            api->coverArt_unref(cover_image);
        }
        cover_image = fw.result();
        fw.cancel();
    }
    if (!cover_image) {
        cover_image = cover_default;
    }
    // uses cover_image, updates size based on cover_size
    if (cover_image) {
        cover_display.setPixmap(QPixmap::fromImage(cover_image->scaled(cover_size, Qt::KeepAspectRatio,Qt::SmoothTransformation)));
    }
}

void CoverArt::resizeEvent(QResizeEvent *event) {
    cover_size = QSize(event->size().width() - m.left() - m.right(), event->size().height() - m.top() - m.bottom());
    if (cover_image) {
        refreshCover();
    }
}
