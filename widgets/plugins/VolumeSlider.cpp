#include "VolumeSlider.h"

#include <QDebug>
#include <QStyleOptionSlider>
#include <QToolBar>
#include <QPainter>
#include <QMouseEvent>
#include <math.h>

#define DBAPI (this->api)

VolumeSlider::VolumeSlider(QWidget *parent, DBApi *Api) : QSlider(parent) {
    api = Api;
    setRange(-50, 0);
    //setOrientation(Qt::Vertical);
    setOrientation(Qt::Horizontal);
    setSingleStep(1);
    setPageStep(1);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    //setMinimumHeight(27)

    // 17 bars default (TODO hardcoded)
    setMinimumWidth(76);
    setMinimumHeight(27);

    // TODO
    setStyleSheet(".QSlider {max-width: 1px;}");

    // DBApi links

    setValue(DBAPI->playback.getVolume());
    setToolTip(QString("%1dB").arg(value()));
    // API -> SLIDER
    connect(&DBAPI->playback, &PlaybackControl::volumeChanged, this,
            [this]() {
        setValue(DBAPI->playback.getVolume());
        setToolTip(QString("%1dB").arg(value()));
    });
    // SLIDER -> API
    connect(this, &QSlider::valueChanged, &DBAPI->playback, &PlaybackControl::setVolume);
}

QObject * VolumeSlider::constructor(QWidget *parent, DBApi *api) {
    if (!parent) {
        QObject *info = new QObject(nullptr);
        info->setProperty("friendlyName", info->tr("Volume Slider"));
        info->setProperty("internalName", "volumeSlider");
        info->setProperty("widgetType", "toolbar");
        info->setProperty("widgetStyle", "Qt Widgets");
        return info;
    }
    return new VolumeSlider(parent,api);
}

void VolumeSlider::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);

    qreal height_max = 19.0;
    qreal height_min = 3.0;
    qreal bar_width = 3.0;
    // TODO marigin not calculated when drawing percent amount (gets off if you change this)
    qreal bar_marigin = 1.0;
    qreal horizontal_marigin = 4.0;
    qreal vertical_marigin = (height()-height_max)/2;

    // widget is set to be minimum 76px wide, which will generate 17 bars
    qreal bar_amount = floor((this->width()-8.0)/(bar_width+bar_marigin));
    qreal percentage = (value()+50.0)/50.0;

    // Start painting
    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing);

    // TODO color hardcoded
    QColor color = QColor(255,255,255); //api->getAccentColor();
    QPen pen(Qt::transparent);
    pen.setWidth(0);
    qp.setPen(pen);
    qp.setBrush(color);

    int i;
    for (i = 0; i < bar_amount; i++) {
        if (abs(bar_amount*percentage) < (static_cast<float>(i)+0.5) || percentage == 0) {
            //qp.setBrush(Qt::white);
            //qp.setBrush(palette().color(QPalette::Inactive,QPalette::Highlight));
            qp.setBrush(palette().color(QPalette::Disabled,QPalette::WindowText));
        }
        qreal bar_height;
        (bar_amount == 17) ? bar_height = height_min + i // 1px increase on default (no smoothing)
                           : bar_height = i/bar_amount * (height_max-height_min) + height_min;
        qreal bar_xpos = i*(bar_width+bar_marigin) + horizontal_marigin;
        qreal bar_ypos = height_max + vertical_marigin - bar_height;
        QRectF rectangle(bar_xpos, bar_ypos, bar_width, bar_height);
        qp.drawRect(rectangle);
    }

    // TODO allow default slider
    //QSlider::paintEvent(e);
}

void VolumeSlider::mouseMoveEvent(QMouseEvent *ev) {
    int margin = 4;
    qDebug() << "MOVE:" << ev->pos();
    float value = (float) (ev->pos().x()-margin) / (float) (this->width()- 2*margin);
    qDebug() << value << ",,," << value;
    value = value * 50.0 - 50;
    qDebug() << "VALUE: " << value;
    DBAPI->playback.setVolume(value);
    return;
}

void VolumeSlider::mousePressEvent ( QMouseEvent * event ) {
    mouseMoveEvent(event);
    return;
}
