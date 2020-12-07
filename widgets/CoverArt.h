#ifndef COVERARTWIDGET_H
#define COVERARTWIDGET_H

#include <QLabel>
#include <QLayout>
#include "DBApi.h"

class CoverArt : public QWidget, public DBWidget{
    Q_OBJECT

public:
    CoverArt(QWidget *parent = nullptr, DBApi *api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

public slots:
    void currCoverChanged(QImage *);
private:
    void setCover(QImage *image);
    virtual void resizeEvent(QResizeEvent *event);
    QHBoxLayout layout;
    QLabel cover_display;
    QImage *cover_image = nullptr;
    QMargins m;
};

#endif // COVERARTWIDGET_H
