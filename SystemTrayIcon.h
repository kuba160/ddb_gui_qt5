#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>
#include <QWheelEvent>

class SystemTrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit SystemTrayIcon(QObject *parent = 0);

protected:
    virtual bool event(QEvent *event);

signals:
    void wheeled(int);
};

#endif // SYSTEMTRAYICON_H
