#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>

class VolumeSlider : public QSlider {
    Q_OBJECT

public:
    VolumeSlider(QWidget *parent = 0);

    void setValue(int value);

protected slots:
    void onValueChanged(int value);
};

#endif // VOLUMESLIDER_H
