#ifndef PLAYBACKBUTTONS_H
#define PLAYBACKBUTTONS_H

#include <QToolBar>
#include <QStyle>
#include "DBApi.h"

typedef struct button_s{
    QString name;
    QStyle::StandardPixmap icon;
    const char *slot;
} button_t;

class PlaybackButtons : public QToolBar, public DBWidget {
    Q_OBJECT
public:
    PlaybackButtons(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api =nullptr);
};

#endif // PLAYBACKBUTTONS_H
