#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <QSlider>
//#include "QtGui.h"
#include "DBApi.h"
#include <QMouseEvent>


#define SEEK_SCALE 10

class SeekSlider : public QSlider, public DBWidget {
    Q_OBJECT

public:
    SeekSlider(QWidget *parent = 0, DBApi *api = 0);
    ~SeekSlider();

    static QWidget  *constructor(QWidget *parent = nullptr, DBApi *api = nullptr);

    QSize sizeHint() const;
protected:
    bool event(QEvent *event);
    void paintEvent(QPaintEvent *e);

protected slots:
    void mouseReleaseEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);

private slots:
    void onFrameUpdate();
    void onPlaybackStop();
    void onPlaybackStart();

private:
    int pos(QMouseEvent *ev) const;
    bool activateNow;
};

#endif // SEEKSLIDER_H
