#ifndef VOLUMESLIDER_H
#define VOLUMESLIDER_H

#include <QSlider>
#include "DBApi.h"

class VolumeSlider : public QSlider, public DBWidget {
    Q_OBJECT

public:
    VolumeSlider(QWidget *parent = nullptr, DBApi *api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);
    void setValue(int value);
    void setValue(float value);

protected:
    void paintEvent(QPaintEvent *e);

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *ev);
    float Volume;

public slots:
    // called when value changes because of slider
    void onSliderValueChanged(int value);
    // called when value changes because of deadbeef (other plugin etc.)
    void onDeadbeefValueChanged();
    // increase / decrease volume by x amount
    void adjustVolume(float value);
signals:
    // emitted when value got changed by slider
    void volumeChanged(float volume);

};

#endif // VOLUMESLIDER_H
