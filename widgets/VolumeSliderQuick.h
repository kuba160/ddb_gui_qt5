#ifndef VOLUMESLIDERQUICK_H
#define VOLUMESLIDERQUICK_H

#include <QQuickWidget>
#include "DBApi.h"

class VolumeSliderQuick : public QQuickWidget, public DBWidget {
    Q_OBJECT

public:
    VolumeSliderQuick(QWidget *parent = nullptr, DBApi *api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;
};

#endif // VOLUMESLIDERQUICK_H
