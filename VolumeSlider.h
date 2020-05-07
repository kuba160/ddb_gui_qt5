#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include "DBApi.h"

class VolumeSlider : public QSlider, public DBToolbarWidget {
    Q_OBJECT

public:
    VolumeSlider(QWidget *parent = nullptr, DBApi *api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);
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
