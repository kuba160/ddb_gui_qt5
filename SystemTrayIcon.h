#ifndef SYSTEMTRAYICON_H
#define SYSTEMTRAYICON_H

#include <QSystemTrayIcon>
#include <QWheelEvent>

class SystemTrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit SystemTrayIcon(QObject *parent = 0);


protected:
    bool event(QEvent *event) override;

private:
    QWidget *window;
public slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);

signals:
    void singleClick();
    void doubleClick();
    void middleClick();
    void wheelScroll(int);
};

#endif // SYSTEMTRAYICON_H
