#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include <QMouseEvent>

class VolumeSlider : public QSlider {
    Q_OBJECT

public:
    VolumeSlider(QWidget *parent = 0);

    void setValue(int value);

protected slots:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
};

#endif // VOLUMESLIDER_H
