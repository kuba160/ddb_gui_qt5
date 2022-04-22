#include "VolumeSlider.h"

#include "QtGuiSettings.h"
#include "QtGui.h"

#include "DBApi.h"
#include "MainWindow.h"

#include <QDebug>
#include <QStyleOptionSlider>

VolumeSlider::VolumeSlider(QWidget *parent, DBApi *api) : QSlider(parent), DBWidget(parent, api) {
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
    //setStyleSheet(".QSlider {max-width: 1px;}");

    // DBApi links
    Volume = api->getVolume();
    QSlider::setValue(Volume);
    setToolTip(QString("%1dB") .arg (Volume));
    // API -> SLIDER
    connect(api, SIGNAL(volumeChanged()), this, SLOT(onDeadbeefValueChanged()));
    // SLIDER -> API
    connect(this, SIGNAL(volumeChanged(float)), api, SLOT(setVolume(float)));
    // SLIDER INTERNAL
    connect(this, SIGNAL(sliderMoved(int)), this, SLOT(onSliderValueChanged(int)));
}

QWidget *VolumeSlider::constructor(QWidget *parent, DBApi *api) {
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
    qreal vertical_marigin = 4.0;

    // widget is set to be minimum 76px wide, which will generate 17 bars
    qreal bar_amount = floor((this->width()-8.0)/(bar_width+bar_marigin));
    qreal percentage = (value()+50.0)/50.0;

    // Start painting
    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing);

    // TODO color hardcoded
    QColor color = api->getAccentColor();
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

void VolumeSlider::setValue(int value) {
    QSlider::setValue(value);
    Volume = value;
    setToolTip(QString("%1dB") .arg (Volume));
    emit volumeChanged(value);
}

void VolumeSlider::setValue(float value) {
    QSlider::setValue(round(value));
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
        //double halfHandleWidth = 0.0;
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
    update();

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
    setToolTip(QString("%1dB") .arg (Volume));
    emit volumeChanged(value);
}

void VolumeSlider::onDeadbeefValueChanged() {
    Volume = api->getVolume();
    QSlider::setValue(Volume);
    // do not emit signal back
}

void VolumeSlider::adjustVolume(float value) {
    setValue(Volume + value);
}
