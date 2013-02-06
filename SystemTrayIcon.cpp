#include "SystemTrayIcon.h"

SystemTrayIcon::SystemTrayIcon(QObject *parent) : QSystemTrayIcon(parent) {
}

bool SystemTrayIcon::event(QEvent *event) {
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheelevent = (QWheelEvent *)event;
        emit wheeled(wheelevent->delta());
        event->accept();
        return true;
    }
    else
        return QSystemTrayIcon::event(event);
}
