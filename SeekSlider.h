#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <QSlider>
//#include "QtGui.h"
#include "DBApi.h"
#include <QMouseEvent>


#define SEEK_SCALE 10

class SeekSlider : public QSlider, public DBToolbarWidget {
    Q_OBJECT

public:
    SeekSlider(QWidget *parent = 0, DBApi *api = 0);
    ~SeekSlider();
    static QWidget  *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);
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
