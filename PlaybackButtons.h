#ifndef PLAYBACKBUTTONS_H
#define PLAYBACKBUTTONS_H

#include <QWidget>
#include <QStyle>
#include <QToolBar>
#include "DBApi.h"

namespace Ui {
    class PlaybackButtons;
}

typedef struct button_s{
    QString name;
    QStyle::StandardPixmap icon;
    const char *slot;
    //char slot[255];
} button_t;

class PlaybackButtons : public QWidget, public DBToolbarWidget {
    Q_OBJECT
public:
    PlaybackButtons(QWidget *parent = nullptr, DBApi *Api = nullptr);
    static QToolBar *constructorToolbar (QWidget *parent = nullptr, DBApi *Api =nullptr);
    static void defaultButtonsAdd (QToolBar *, DBApi *);
private:
    Ui::PlaybackButtons *ui;

signals:

};

#endif // PLAYBACKBUTTONS_H
