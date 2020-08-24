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
    //setFixedWidth(80);
    setMinimumWidth(72);
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
    vslider->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    //vslider->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    return new VolumeSlider(parent,api);
}

QSize VolumeSlider::sizeHint() const
{
    return QSize(17*4+2,27);
}

void VolumeSlider::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);

    qreal height_max = 19.0;
    qreal height_min = 3.0;
    qreal bar_width = 3.0;
    qreal bar_marigin = 1.0;
    qreal horizontal_marigin = 4.0;

    qreal bar_amount = 17;//this->width()/(bar_width+bar_marigin);

    qreal percentage = (value()+50.0)/50.0;

    qDebug() << "VolumeSlider with" << bar_amount << "bars." << endl;
    //setFixedWidth(bar_marigin + bar_amount*(bar_width+bar_marigin));
    setFixedHeight(height_max+2*horizontal_marigin);

    qreal total_pixels = bar_marigin + bar_amount*(bar_width+bar_marigin);

    // Start painting
    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing);
    QColor blue(43,127,186);
    QPen pen(Qt::transparent);
    pen.setWidth(0);
    qp.setPen(pen);
    qp.setBrush(blue);
    qDebug() << "Percentage:" << percentage;
    qDebug() << "Value:" <<value();

    int i;
    for (i = 0; i < bar_amount; i++) {
        if (bar_amount*percentage+1.0 < i || percentage == 0) {
            //qDebug() << "White bars from" << i;
            qp.setBrush(Qt::white);
        }
        QRectF rectangle(2.0+i*(bar_width+bar_marigin),height_max+horizontal_marigin-i-height_min,bar_width, height_min+i);
        qp.drawRect(rectangle);
    }
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
        //double halfHandleWidth = (0.5 * sr.width()) + 0.5; // Correct rounding
        double halfHandleWidth = 0.0;
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
