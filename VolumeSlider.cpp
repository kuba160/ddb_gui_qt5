#include "VolumeSlider.h"

#include "QtGuiSettings.h"
#include "QtGui.h"

VolumeSlider::VolumeSlider(QWidget *parent) : QSlider(parent) {
    setRange(-50, 0);
    setOrientation(Qt::Horizontal);
    setFixedWidth(80);
    setValue(DBAPI->volume_get_db());
    // make all events move volume by 1 dB
    setSingleStep(1);
    setPageStep(1);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
}

void VolumeSlider::setValue(int value) {
    QSlider::setValue(value);
    DBAPI->volume_set_db(value);
}

void VolumeSlider::onValueChanged(int value) {
    setValue(value);
}
