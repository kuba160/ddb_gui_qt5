#include "VolumeSlider.h"

#include "QtGuiSettings.h"
#include "QtGui.h"

#define SIGNUM(x) ((x > 0) - (x < 0))

VolumeSlider::VolumeSlider(QWidget *parent) : QSlider(parent) {
    setRange(-50, 0);
    setOrientation(Qt::Horizontal);
    setFixedWidth(80);
    setValue(DBAPI->volume_get_db());
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
}


void VolumeSlider::mousePressEvent(QMouseEvent *ev) {
    mouseMoveEvent(ev);
}

void VolumeSlider::mouseMoveEvent(QMouseEvent *ev) {
    int val = minimum() - ((float)ev->x() / this->width()) * (minimum());
    if(val<=maximum() && val>=minimum()) {
        setValue(val);
    }
}

void VolumeSlider::setValue(int value) {
    QSlider::setValue(value);
    DBAPI->volume_set_db(value);
}
