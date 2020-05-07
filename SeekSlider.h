#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <QSlider>
//#include "QtGui.h"
#include "DBApi.h"
#include <QMouseEvent>


#define SEEK_SCALE 10

class SeekSlider : public QSlider {
    Q_OBJECT

public:
    SeekSlider(QWidget *parent = 0, DBApi *api = 0);
    ~SeekSlider();

protected:
    bool event(QEvent *event);

protected slots:
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

private slots:
    void onFrameUpdate();
    void onPlaybackStop();
    void onPlaybackStart();

private:
    int pos(QMouseEvent *ev) const;
    bool activateNow;
    DBApi *api;
};

#endif // SEEKSLIDER_H
