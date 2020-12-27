#include "SystemTrayIcon.h"

#include "DBApi.h"

#define SIGNUM(x) ((x > 0) - (x < 0))

SystemTrayIcon::SystemTrayIcon(QObject *parent) : QSystemTrayIcon(parent) {
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onActivated(QSystemTrayIcon::ActivationReason)));

}

bool SystemTrayIcon::event(QEvent *event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelevent = (QWheelEvent *)event;
        // TODO
        //emit wheelScroll(SIGNUM(wheelevent->delta()));
        event->accept();
        return true;
    }
    else
        return QSystemTrayIcon::event(event);
}

void SystemTrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {

    if (reason == QSystemTrayIcon::Trigger) {
        emit singleClick();
    }
    else if (reason == QSystemTrayIcon::DoubleClick) {
        emit doubleClick();
    }
    else if (reason == QSystemTrayIcon::MiddleClick) {
        emit middleClick();
    }
}
