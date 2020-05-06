#include "VolumeSlider.h"

#include "QtGuiSettings.h"
#include "QtGui.h"

#include "DBApi.h"
#include "MainWindow.h"

VolumeSlider::VolumeSlider(QWidget *parent, DBApi *api) : QSlider(parent) {
    setRange(-50, 0);
    setOrientation(Qt::Horizontal);
    setFixedWidth(80);
    setSingleStep(1);
    setPageStep(1);
    setFocusPolicy(Qt::NoFocus);

    // DBApi links
    Volume = api->getVolume();
    // API -> SLIDER
    connect(api, SIGNAL(volumeChanged(int)), this, SLOT(onDeadbeefValueChanged(int)));
    // SLIDER -> API
    connect(this, SIGNAL(volumeChanged(int)), api, SLOT(setVolume(int)));
    // SLIDER INTERNAL
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

}

void VolumeSlider::setValue(int value) {
    QSlider::setValue(value);
    Volume = value;
    emit volumeChanged(value);
}

void VolumeSlider::mousePressEvent ( QMouseEvent * event ) {
    if (event->button() == Qt::LeftButton) {
        if (orientation() == Qt::Vertical)
            setValue(minimum() + ((maximum()-minimum()) * (height()-event->y())) / height() ) ;
        else
            setValue(minimum() + ((maximum()-minimum()) * event->x()) / width() ) ;
        event->accept();
        QSlider::mousePressEvent(event);
    }
}

void VolumeSlider::onSliderValueChanged(int value) {
    // QSlider::setValue(value);
    Volume = value;
    emit volumeChanged(value);
}

void VolumeSlider::onDeadbeefValueChanged(int value) {
    QSlider::setValue(value);
    Volume = value;
    // do not emit signal back
}

void VolumeSlider::adjustVolume(int value) {
    setValue(Volume + value);
}
