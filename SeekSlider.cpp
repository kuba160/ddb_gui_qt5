#include "SeekSlider.h"

#include "QtGuiSettings.h"
#include "GuiUpdater.h"

#include <QSizePolicy>

#undef DBAPI
#define DBAPI api->deadbeef

SeekSlider::SeekSlider(QWidget *parent, DBApi *Api) : QSlider(parent), DBToolbarWidget(parent, Api) {
    activateNow = false;
    setRange(0, 100 * SEEK_SCALE);
    setOrientation(Qt::Horizontal);
    api = Api;
    connect(GuiUpdater::Instance(), SIGNAL(frameUpdate()), this, SLOT(onFrameUpdate()));

    connect(api, SIGNAL(playbackStarted()),this,SLOT(onPlaybackStart()));
    connect(api, SIGNAL(playbackStopped()),this,SLOT(onPlaybackStop()));
    if (api->getInternalState() == DDB_PLAYBACK_STATE_STOPPED)
        this->setEnabled(false);
}

SeekSlider::~SeekSlider() {
}

QWidget * SeekSlider::constructor(QWidget *parent, DBApi *api) {
    SeekSlider *slider = new SeekSlider(parent, api);
    slider->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
    return slider;
}

bool SeekSlider::event(QEvent *event) {
    if (event->type() == QEvent::Wheel) {
            return true;
    }
    return QWidget::event(event);
}

void SeekSlider::mouseReleaseEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        DBAPI->playback_set_pos(value() / SEEK_SCALE);
        activateNow = false;
    }
}


void SeekSlider::mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        activateNow = true;
        setValue(pos(ev));
    }
}

void SeekSlider::mouseMoveEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        setValue(pos(ev));
    }
}

void SeekSlider::onFrameUpdate() {
    if (activateNow) return;
    if (isHidden() || parentWidget()->isHidden())
        return;
    int output_state = api->getOutputState();
    if (output_state == DDB_PLAYBACK_STATE_PAUSED || output_state == DDB_PLAYBACK_STATE_PLAYING) {
        setValue(DBAPI->playback_get_pos() * SEEK_SCALE);
    }
}

void SeekSlider::onPlaybackStop() {
    this->setEnabled(false);
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
