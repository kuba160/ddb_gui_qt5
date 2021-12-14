#ifndef SEEKSLIDERQUICK_H
#define SEEKSLIDERQUICK_H

#include <QQuickWidget>
#include "DBApi.h"

class SeekSliderQuick : public QQuickWidget, public DBWidget {
    Q_OBJECT

public:
    SeekSliderQuick(QWidget *parent = nullptr, DBApi *api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;
};

#endif // SEEKSLIDERQUICK_H
