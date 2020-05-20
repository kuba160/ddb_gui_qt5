#ifndef PLAYBACKBUTTONS_H
#define PLAYBACKBUTTONS_H

#include <QWidget>
#include <QToolBar>
#include "DBApi.h"

namespace Ui {
    class PlaybackButtons;
}

class PlaybackButtons : public QWidget, public DBToolbarWidget {
    Q_OBJECT
public:
    PlaybackButtons(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QToolBar *constructorToolbar (QWidget *parent = nullptr, DBApi *Api =nullptr);

private:
    Ui::PlaybackButtons *ui;

signals:

};

#endif // PLAYBACKBUTTONS_H
