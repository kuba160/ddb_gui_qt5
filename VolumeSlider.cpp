#include "VolumeSlider.h"

#include "QtGuiSettings.h"
#include "QtGui.h"

#include "DBApi.h"
#include "MainWindow.h"

#include <QDebug>
#include <QStyleOptionSlider>


VolumeSlider::VolumeSlider(QWidget *parent, DBApi *api) : QSlider(parent), DBToolbarWidget(parent, api) {
    setRange(-50, 0);
    //setOrientation(Qt::Vertical);
    setOrientation(Qt::Horizontal);
    setFixedWidth(80);
    setSingleStep(1);
    setPageStep(1);
    setFocusPolicy(Qt::NoFocus);

    // DBApi links
    Volume = api->getVolume();
    QSlider::setValue(Volume);
    setToolTip(QString("%1dB") .arg (Volume));
    // API -> SLIDER
    connect(api, SIGNAL(volumeChanged(int)), this, SLOT(onDeadbeefValueChanged(int)));
    // SLIDER -> API
    connect(this, SIGNAL(volumeChanged(int)), api, SLOT(setVolume(int)));
    // SLIDER INTERNAL
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));

}

QWidget *VolumeSlider::constructor(QWidget *parent, DBApi *api) {
    VolumeSlider *vslider = new VolumeSlider(parent, api);
    //vslider->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    vslider->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    return new VolumeSlider(parent,api);
}

void VolumeSlider::setValue(int value) {
    QSlider::setValue(value);
    Volume = value;
    setToolTip(QString("%1dB") .arg (Volume));
    emit volumeChanged(value);
}

void VolumeSlider::mousePressEvent ( QMouseEvent * event ) {
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    if (event->button() == Qt::LeftButton && sr.contains(event->pos()) == false) {
    int newVal;
    if (orientation() == Qt::Vertical) {
        newVal = minimum() + ((maximum()-minimum()) * (height()-event->y())) / height();
    }
    else {
        double halfHandleWidth = (0.5 * sr.width()) + 0.5; // Correct rounding
        int adaptedPosX = event->x();
        if (adaptedPosX < halfHandleWidth)
            adaptedPosX = halfHandleWidth;
        if (adaptedPosX > width() - halfHandleWidth)
            adaptedPosX = width() - halfHandleWidth;
        // get new dimensions accounting for slider handle width
        double newWidth = (width() - halfHandleWidth) - halfHandleWidth;
        double normalizedPosition = (adaptedPosX - halfHandleWidth)  / newWidth ;
        newVal = minimum() + ((maximum()-minimum()) * normalizedPosition);
    }
    if (invertedAppearance() == true)
      VolumeSlider::setValue( maximum() - newVal );
    else
      VolumeSlider::setValue(newVal);

    event->accept();
    }
    QSlider::mousePressEvent(event);
}

void VolumeSlider::mouseReleaseEvent ( QMouseEvent * event ) {
    if (event->button() == Qt::LeftButton) {
        setToolTip(QString("%1dB") .arg (Volume));
    }
    QSlider::mouseReleaseEvent(event);
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
