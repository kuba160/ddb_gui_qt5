#ifndef COVERARTQUICK_H
#define COVERARTQUICK_H

#include <QQuickImageProvider>

#include "DBApi.h"

class CoverArtQuick : public QQuickImageProvider {
public:
    CoverArtQuick(DBApi *Api);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

protected:
    DBApi *api;
};

#endif // COVERARTQUICK_H
