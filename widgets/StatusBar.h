#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QStatusBar>
#include <QLabel>
#include <QTimer>
#include "DBApi.h"

class StatusBar : public QStatusBar, DBWidget {
    Q_OBJECT
public:
    StatusBar(QWidget *parent = nullptr, DBApi *api = nullptr);
    ~StatusBar();
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

    void format_timestr(char *buf, int sz, float time);
protected:
    QLabel *label;
    QTimer timer;
    char *track_script;

public slots:
    void update();
};

#endif // STATUSBAR_H
