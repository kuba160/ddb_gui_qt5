#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include <dbapi/DBApi.h>

class VolumeSlider : public QSlider {
    Q_OBJECT
    DBApi *api;
public:
    VolumeSlider(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QObject *constructor(QWidget *parent = nullptr, DBApi *Api =nullptr);

protected:
    void paintEvent(QPaintEvent *e);

private:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
};

#endif // VOLUMESLIDER_H
