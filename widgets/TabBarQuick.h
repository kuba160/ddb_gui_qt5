#ifndef TABBARQUICK_H
#define TABBARQUICK_H

#include <QQuickWidget>
#include "DBApi.h"

class TabBarQuick : public QQuickWidget, public DBWidget {
    Q_OBJECT

public:
    TabBarQuick(QWidget *parent = nullptr, DBApi *api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);

protected:
    void resizeEvent(QResizeEvent *) override;
};

#endif // TABBARQUICK_H
