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
    if (cover_image != image) {
        cover_image = image;
        if (cover_image) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
            cover_display.setPixmap(QPixmap::fromImage(cover_image->scaled(this->size().shrunkBy(m),Qt::KeepAspectRatio,Qt::SmoothTransformation)));
#else
            QSize size = QSize(this->size().width() - (m.left()+m.right()), this->size().height() -(m.top()+m.bottom()));
            cover_display.setPixmap(QPixmap::fromImage(cover_image->scaled(size, Qt::KeepAspectRatio,Qt::SmoothTransformation)));
#endif
        }
    }
}

void CoverArt::resizeEvent(QResizeEvent *event) {
    if (cover_image) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        cover_display.setPixmap(QPixmap::fromImage(cover_image->scaled(event->size().shrunkBy(m),Qt::KeepAspectRatio,Qt::SmoothTransformation)));
#else
        QSize size = QSize(event->size().width() -(m.left()+m.right()), event->size().height() -(m.top()+m.bottom()));
        cover_display.setPixmap(QPixmap::fromImage(cover_image->scaled(size, Qt::KeepAspectRatio,Qt::SmoothTransformation)));
#endif
    }
}
