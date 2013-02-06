#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include <QTime>
#include <QHash>

class StatusBar : public QStatusBar {
    Q_OBJECT

public:
    StatusBar(QWidget *parent = 0);
    ~StatusBar();
    
    void loadConfig();
    void saveConfig();
    
private:
    QLabel statusLabel;
    QTime lastBitrateUpdate;

    QString playingFormat;
    QString pausedFormat;
    QString stoppedFormat;

    const QString getTotalTimeString();

    QHash<QString, QString> values;

private Q_SLOTS:
    void showPopUp(QPoint point);
    void onFrameUpdate();
    void setFormat();
};

#endif // STATUSBAR_H
