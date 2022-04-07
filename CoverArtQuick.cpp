#include "CoverArtQuick.h"

CoverArtQuick::CoverArtQuick(DBApi *Api) : QQuickImageProvider(QQuickImageProvider::Image) {
    api = Api;
}

QImage CoverArtQuick::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    QStringList l = id.split('/', Qt::SkipEmptyParts);
    if (l.length() < 2) {
        qDebug() << "CoverArtQuick: id is not in playlist/index format!";
        return QImage();
    }

    if (l[0] == "current") {
        DB_playItem_t *it = DBAPI->streamer_get_playing_track();
        if (it) {
            if (!api->isCoverArtCached(it, requestedSize)) {
                QFuture<QImage *> f = api->requestCoverArt(it, requestedSize);
                f.waitForFinished();
                QImage *img = f.result();
                DBAPI->pl_item_unref(it);
                if  (img) {
                    QImage ret(*img);
                    api->coverArt_unref(img);
                    return ret;
                }
            }
            else {
                QImage *img = api->getCoverArt(it, requestedSize);
                DBAPI->pl_item_unref(it);
                if  (img) {
                    QImage ret(*img);
                    api->coverArt_unref(img);
                    return ret;
                }
            }
        }
    }

    bool ok = false;
    int plt_num = l[0].toInt(&ok);
    if (!ok) {
        qDebug() << "CoverArtQuick: couldn't translate playlist to integer!";
        return QImage();
    }
    int it_num = l[1].toInt(&ok);
    if (!ok) {
        qDebug() << "CoverArtQuick: couldn't translate index to integer!";
        return QImage();
    }

    ddb_playlist_t *plt = DBAPI->plt_get_for_idx(plt_num);
    if (!plt) {
        qDebug() << "CoverArtQuick: couldn't get playlist number" << plt_num;
        return QImage();
    }
    DB_playItem_t *it = DBAPI->plt_get_item_for_idx(plt, it_num, PL_MAIN);
    if (!it) {
        DBAPI->plt_unref(plt);
        qDebug() << "CoverArtQuick: couldn't get playitem number" << it_num;
        return QImage();
    }

    QImage *img;
    if (api->isCoverArtCached(it, requestedSize)) {
        img = api->getCoverArt(it, requestedSize);
    }
    else {
        QFuture<QImage *> f = api->requestCoverArt(it, requestedSize);
        f.waitForFinished();
        img = f.result();
    }
    DBAPI->pl_item_unref(it);
    DBAPI->plt_unref(plt);
    if (img) {
        QImage ret(*img);
        *size = requestedSize;
        api->coverArt_unref(img);
        return ret;
    }
    qDebug() << "CoverArtQuick: couldn't receive any cover";
    return QImage();
}
