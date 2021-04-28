#ifndef COVERARTWIDGET_H
#define COVERARTWIDGET_H

#include <QLabel>
#include <QLayout>
#include <QFutureWatcher>

#include "DBApi.h"

class CoverArt : public QWidget, public DBWidget{
    Q_OBJECT

public:
    CoverArt(QWidget *parent = nullptr, DBApi *api = nullptr);
    ~CoverArt();
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

private slots:
    void onTrackChanged(DB_playItem_t*,DB_playItem_t*);
    void onPlaybackStopped();
    void refreshCover();

private:
    virtual void resizeEvent(QResizeEvent *event);
    QHBoxLayout layout;
    QLabel cover_display;
    QImage *cover_image = nullptr;
    QImage *cover_default = nullptr;
    QSize cover_size;
    QMargins m;

    QFutureWatcher<QImage*> fw;
};

#endif // COVERARTWIDGET_H

