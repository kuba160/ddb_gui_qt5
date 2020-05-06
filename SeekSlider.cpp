#include "SeekSlider.h"

#include "QtGuiSettings.h"
#include "GuiUpdater.h"

SeekSlider::SeekSlider(QWidget *parent, DBApi *api) : QSlider(parent) {
    activateNow = false;
    setRange(0, 100 * SEEK_SCALE);
    setOrientation(Qt::Horizontal);
    Api = api;
    connect(GuiUpdater::Instance(), SIGNAL(frameUpdate()), this, SLOT(onFrameUpdate()));
}

SeekSlider::~SeekSlider() {
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
    setValue(pos(ev));
}

void SeekSlider::onFrameUpdate() {
    if (activateNow) return;
    if (isHidden() || parentWidget()->isHidden())
        return;
    int output_state = Api->getOutputState();
    if (output_state == DDB_PLAYBACK_STATE_PAUSED || output_state == DDB_PLAYBACK_STATE_PLAYING) {
        setValue(DBAPI->playback_get_pos() * SEEK_SCALE);
    }
}

int SeekSlider::pos(QMouseEvent *ev) const {
    int val = ((float)ev->x() / this->width()) * maximum();
    if(val >= maximum()) return maximum();
    if(val <= minimum()) return minimum();
    return val;
}
