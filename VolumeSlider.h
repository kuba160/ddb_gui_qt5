#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include "DBApi.h"

class VolumeSlider : public QSlider {
    Q_OBJECT

public:
    VolumeSlider(QWidget *parent = 0, DBApi *api = 0);

    void setValue(int value);

private:
    void mousePressEvent(QMouseEvent *event);
    int Volume;

public slots:
    // called when value changes because of slider
    void onSliderValueChanged(int value);
    // called when value changes because of deadbeef (other plugin etc.)
    void onDeadbeefValueChanged(int value);
    // increase / decrease volume by x amount
    void adjustVolume(int value);
signals:
    // emitted when value got changed by slider
    void volumeChanged(int volume);

};

#endif // VOLUMESLIDER_H
