#ifndef COVERARTWIDGET_H
#define COVERARTWIDGET_H

#include <QLabel>
#include <QLayout>
#include "DBApi.h"

class CoverArt : public QWidget, public DBWidget{
    Q_OBJECT

public:
    CoverArt(QWidget *parent = nullptr, DBApi *api = nullptr);
    ~CoverArt();
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

public slots:
    void currCoverChanged(QImage *);
private:
    void setCover(QImage *image);
    virtual void resizeEvent(QResizeEvent *event);
    QHBoxLayout *layout = nullptr;
    QLabel *cover_display = nullptr;
    QImage *cover_image = nullptr;
    QMargins *m;
};

#endif // COVERARTWIDGET_H

