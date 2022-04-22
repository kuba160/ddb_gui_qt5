#include "SeekSlider.h"

#include "QtGuiSettings.h"

#include <QSizePolicy>
#include <QStyleOptionSlider>
#include <QLabel>
#include <QPainter>
#include <QtMath>
#include <QToolBar>

#undef DBAPI
#define DBAPI api->deadbeef

SeekSlider::SeekSlider(QWidget *parent, DBApi *Api) : QSlider(parent), DBWidget(parent, Api) {
    activateNow = false;
    setRange(0, 100 * SEEK_SCALE);
    setOrientation(Qt::Horizontal);

    connect(api, SIGNAL(playbackStarted()),this,SLOT(onPlaybackStart()));
    connect(api, SIGNAL(playbackStopped()),this,SLOT(onPlaybackStop()));
    if (api->getInternalState() == DDB_PLAYBACK_STATE_STOPPED)
        this->setEnabled(false);

    connect(&timer, SIGNAL(timeout()), this, SLOT(repaint()));
    timer.start(100);
}

SeekSlider::~SeekSlider() {
}

QSize SeekSlider::sizeHint() const
{
    return QSize(maximumWidth()/2, 20);
}

QWidget *SeekSlider::constructor(QWidget *parent, DBApi *api) {
    QToolBar *tb = static_cast<QToolBar *>(parent);
    tb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    tb->setContextMenuPolicy(Qt::PreventContextMenu);
    tb->setFloatable(false);
    SeekSlider *slider = new SeekSlider(tb, api);
    slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return slider;
}



bool SeekSlider::event(QEvent *event) {
    if (event->type() == QEvent::Wheel) {
            return true;
    }
    return QWidget::event(event);
}

void SeekSlider::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);

    qreal slider_width = this->width()-4.0;
    qreal slider_height = 8.0;
    qreal slider_xpos = 2.0;
    qreal slider_ypos = this->height()/2-4.0;
    float percentage = DBAPI->playback_get_pos()/100.0;

    // TODO: Set colors from theme
    QColor color = api->getAccentColor();
    QPen pen(color);
    pen.setWidth(2);
    pen.setBrush(color);

    QRectF rectangle(slider_xpos, slider_ypos, slider_width, slider_height);
    QRectF progress_mask(slider_xpos, 0.0, qFloor(slider_width * percentage), this->height());
    qreal rect_xrad = 5.0;
    qreal rect_yrad = 4.0;

    // Start painting
    QPainter qp(this);
    qp.setRenderHint(QPainter::Antialiasing);
    qp.setPen(pen);
    qp.setBrush(color);

    // draw a full slider, but clip it according to progress
    qp.setClipping(true);
    qp.setClipRect(progress_mask);
    qp.drawRoundedRect(rectangle, rect_xrad, rect_yrad);

    // draw a full slider, but this time empty inside
    qp.setClipping(false);
    qp.setBrush(Qt::transparent);
    qp.drawRoundedRect(rectangle, rect_xrad, rect_yrad);

    // draw fade line to make progress smooth (don't draw on edges)
    const char no_line_zone = 7;
    if (qCeil(slider_width * percentage) > no_line_zone && qCeil(slider_width * percentage) < (slider_width - no_line_zone)) {
        qreal line_xpos = qCeil(slider_width * percentage + 1.0);
        qreal pixel_edge_fade = (slider_width * percentage) - qFloor(slider_width * percentage); // 0..1
        int line_color = qFloor(255*pixel_edge_fade);
        QColor color_fade(color);
        color_fade.setAlpha(line_color);
        QPen pen2(color_fade);
        pen2.setWidthF(1.5);
        qp.setPen(pen2);
        qp.drawLine(line_xpos, slider_ypos, line_xpos, slider_ypos + slider_height);
    }
}

void SeekSlider::mouseReleaseEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton || ev->button() == Qt::RightButton) {
        DBAPI->playback_set_pos(((float)value() / (float)SEEK_SCALE));
        activateNow = false;
    }
    QSlider::mouseReleaseEvent(ev);
}

void SeekSlider::mousePressEvent ( QMouseEvent * event ) {
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    if ((event->button() == Qt::LeftButton || event->button() == Qt::RightButton) && sr.contains(event->pos()) == false) {
        int newVal;
        if (orientation() == Qt::Vertical) {
            newVal = minimum() + ((maximum()-minimum()) * (height()-event->y())) / height();
        }
        else {
            // TODO: use when using default qt slider
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
          QSlider::setValue( maximum() - newVal );
        else
          QSlider::setValue(newVal);

        activateNow = true;
        update();
        event->accept();
    }
    QSlider::mousePressEvent(event);
}

void SeekSlider::onFrameUpdate() {
    if (activateNow) return;
    if (isHidden() || parentWidget()->isHidden())
        return;
    int output_state = api->getOutputState();
    if (output_state == DDB_PLAYBACK_STATE_PAUSED || output_state == DDB_PLAYBACK_STATE_PLAYING) {
        QSlider::setValue(DBAPI->playback_get_pos() * SEEK_SCALE);
        update();
    }
}

void SeekSlider::onPlaybackStop() {
    this->setEnabled(false);
    update();
    this->setValue(0);
}

void SeekSlider::onPlaybackStart() {
    this->setEnabled(true);
    update();
}

int SeekSlider::pos(QMouseEvent *ev) const {
    int val = ((float)ev->x() / this->width()) * maximum();
    if(val >= maximum()) return maximum();
    if(val <= minimum()) return minimum();
    return val;
}
