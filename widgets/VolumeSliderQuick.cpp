#include "VolumeSliderQuick.h"

#include "QtGuiSettings.h"
#include "QtGui.h"

#include "DBApi.h"
#include "MainWindow.h"

#include <QDebug>
#include <QStyleOptionSlider>

VolumeSliderQuick::VolumeSliderQuick(QWidget *parent, DBApi *api) : QQuickWidget(parent), DBWidget(parent, api) {
    //setFocusPolicy(Qt::NoFocus);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    //setMinimumHeight(27)

    // 17 bars default (TODO hardcoded)
    setMinimumWidth(76);
    setMinimumHeight(27);

    setWindowFlags(Qt::SplashScreen);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setClearColor(Qt::transparent);
    setResizeMode(QQuickWidget::SizeRootObjectToView);

    QUrl source("qrc:/widgets/VolumeSliderQuick.qml");
    setSource(source);
    //setFormat
    // TODO
    //setStyleSheet(".QSlider {max-width: 1px;}");
/*
    // DBApi links
    Volume = api->getVolume();
    QSlider::setValue(Volume);
    setToolTip(QString("%1dB") .arg (Volume));
    // API -> SLIDER
    connect(api, SIGNAL(volumeChanged(float)), this, SLOT(onDeadbeefValueChanged(float)));
    // SLIDER -> API
    connect(this, SIGNAL(volumeChanged(float)), api, SLOT(setVolume(float)));
    // SLIDER INTERNAL
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    */
}

QWidget *VolumeSliderQuick::constructor(QWidget *parent, DBApi *api) {
    return new VolumeSliderQuick(parent,api);
}
