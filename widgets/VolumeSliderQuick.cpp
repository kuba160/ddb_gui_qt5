#include "VolumeSliderQuick.h"

#include <QDebug>
#include <QQmlContext>
#include <QQuickItem>

#include "DBApi.h"

VolumeSliderQuick::VolumeSliderQuick(QWidget *parent, DBApi *api) : QQuickWidget(parent), DBWidget(parent, api) {
    // Allow resize
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 17 bars default (TODO hardcoded)
    setMinimumWidth(70);
    setMinimumHeight(27);

    // Transparency fix
    setWindowFlags(Qt::SplashScreen);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setClearColor(Qt::transparent);

    // Set API and load widget
    rootContext()->setContextProperty("api", api);
    QUrl source("qrc:/widgets/VolumeSliderQuick.qml");
    setSource(source);
}

QWidget *VolumeSliderQuick::constructor(QWidget *parent, DBApi *api) {
    return new VolumeSliderQuick(parent,api);
}

void VolumeSliderQuick::resizeEvent(QResizeEvent *event) {
    // resize qml widget
    QQuickWidget::resizeEvent(event);
    rootObject()->setSize(event->size());
}

void VolumeSliderQuick::wheelEvent(QWheelEvent *ev) {
    // for some reason wheelEvent is not passed to QQuickWidget
    // increase volume manually
    float volume = api->getVolume();
    const float s = 1.0;
    if (ev->angleDelta().y() > 0) {
        //QMetaObject::invokeMethod(rootObject(), "increase");
        api->setVolume(volume + s > 0.0 ? 0 : volume + s);
    }
    else if (ev->angleDelta().y() < 0) {
        QMetaObject::invokeMethod(rootObject(), "decrease");
        api->setVolume(volume - s < -50.0 ? (float) -50.0 : volume - s);
    }
}
