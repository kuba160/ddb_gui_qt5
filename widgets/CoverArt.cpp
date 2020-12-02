#include "CoverArt.h"
#include "QtGui.h"

CoverArt::CoverArt(QWidget *parent, DBApi *api): QWidget(parent), DBWidget (parent, api) {
    cover_display.setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored);
    cover_display.setAlignment(Qt::AlignHCenter);
    cover_display.setVisible(true);
    setMinimumSize(64,64);
    m = QMargins(10,10,10,10);
    layout.addWidget(&cover_display);
    setLayout(&layout);
    setCover(api->getDefaultCoverArt());
    connect(api,SIGNAL(currCoverChanged(QImage *)),this,SLOT(currCoverChanged(QImage *)));
}

QWidget *CoverArt::constructor(QWidget *parent, DBApi *Api) {
    if (!Api->isCoverArtPluginAvailable()) {
        return new QLabel(QString("Artwork plugin not available"));
    }
    return new CoverArt(parent, Api);
}

void CoverArt::currCoverChanged(QImage *img) {
    setCover(img);
}

void CoverArt::setCover(QImage *image) {
    qDebug() << image << cover_image;
    if (cover_image != image) {
        cover_image = image;
        if (cover_image) {
            cover_display.setPixmap(QPixmap::fromImage(cover_image->scaled(this->size().shrunkBy(m),Qt::KeepAspectRatio,Qt::SmoothTransformation)));
        }
    }
}

void CoverArt::resizeEvent(QResizeEvent *event) {
    if (cover_image) {
        cover_display.setPixmap(QPixmap::fromImage(cover_image->scaled(event->size().shrunkBy(m),Qt::KeepAspectRatio,Qt::SmoothTransformation)));
    }
}
