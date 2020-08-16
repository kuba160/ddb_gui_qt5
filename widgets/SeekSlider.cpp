#include "SeekSlider.h"

#include "QtGuiSettings.h"
#include "GuiUpdater.h"

#include <QSizePolicy>
#include <QStyleOptionSlider>
#include <QLabel>

#undef DBAPI
#define DBAPI api->deadbeef

SeekSlider::SeekSlider(QWidget *parent, DBApi *Api) : QSlider(parent), DBWidget(parent, Api) {
    activateNow = false;
    setRange(0, 100 * SEEK_SCALE);
    setOrientation(Qt::Horizontal);
    connect(GuiUpdater::Instance(), SIGNAL(frameUpdate()), this, SLOT(onFrameUpdate()));

    connect(api, SIGNAL(playbackStarted()),this,SLOT(onPlaybackStart()));
    connect(api, SIGNAL(playbackStopped()),this,SLOT(onPlaybackStop()));
    if (api->getInternalState() == DDB_PLAYBACK_STATE_STOPPED)
        this->setEnabled(false);
}

SeekSlider::~SeekSlider() {
}

QToolBar *SeekSlider::constructorToolbar(QWidget *parent, DBApi *api) {
    QToolBar *tb = new QToolBar(parent);
    SeekSlider *slider = new SeekSlider(tb, api);
    tb->addWidget(slider);
    tb->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    tb->setContextMenuPolicy(Qt::PreventContextMenu);
    return tb;
}

bool SeekSlider::event(QEvent *event) {
    if (event->type() == QEvent::Wheel) {
            return true;
    }
    return QWidget::event(event);
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
          QSlider::setValue( maximum() - newVal );
        else
          QSlider::setValue(newVal);

        activateNow = true;
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
    }
}

void SeekSlider::onPlaybackStop() {
    this->setEnabled(false);
    this->setValue(0);
}

void SeekSlider::onPlaybackStart() {
    this->setEnabled(true);
}

int SeekSlider::pos(QMouseEvent *ev) const {
    int val = ((float)ev->x() / this->width()) * maximum();
    if(val >= maximum()) return maximum();
    if(val <= minimum()) return minimum();
    return val;
}
