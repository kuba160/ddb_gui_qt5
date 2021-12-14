#include "SeekSliderQuick.h"

#include <QDebug>
#include <QQmlContext>
#include <QQuickItem>

#include "DBApi.h"

SeekSliderQuick::SeekSliderQuick(QWidget *parent, DBApi *api) : QQuickWidget(parent), DBWidget(parent, api) {
    // Allow resize
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    // 17 bars default (TODO hardcoded)
    setMinimumWidth(90);
    setMinimumHeight(27);

    // Transparency fix
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    setWindowFlags(Qt::SplashScreen);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setClearColor(Qt::transparent);
#else
    // TODO fix transparency for qt 6
    setWindowFlags(Qt::SplashScreen);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setClearColor(Qt::transparent);
#endif

    // Set API and load widget
    rootContext()->setContextProperty("api", api);
    QUrl source("qrc:/widgets/SeekSliderQuick.qml");
    setSource(source);
}

QWidget *SeekSliderQuick::constructor(QWidget *parent, DBApi *api) {
    return new SeekSliderQuick(parent,api);
}

void SeekSliderQuick::resizeEvent(QResizeEvent *event) {
    QQuickWidget::resizeEvent(event);
    // resize qml widget
    if (rootObject()) {
        rootObject()->setSize(event->size());
    }
}
