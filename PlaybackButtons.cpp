#include "PlaybackButtons.h"

button_t buttons[] = {{QString("stop") , QStyle::SP_MediaStop,  SLOT(stop())},
                      {QString("play") , QStyle::SP_MediaPlay,  SLOT(play())},
                      {QString("pause"), QStyle::SP_MediaPause, SLOT(togglePause())},
                      {QString("prev"),  QStyle::SP_MediaSkipBackward, SLOT(playPrev())},
                      {QString("next"),  QStyle::SP_MediaSkipForward, SLOT(playNext())},
                      {QString(),        QStyle::SP_TitleBarMenuButton, 0}};

PlaybackButtons::PlaybackButtons(QWidget *parent, DBApi *Api) : QToolBar(parent), DBToolbarWidget(parent, Api) {
    // Setup
    this->setIconSize(QSize(16, 16));
    this->setStyleSheet("QToolButton{padding: 6px;}");

    // Add actions
    int i;
    for (i = 0; buttons[i].slot; i++) {
        QAction *a = this->addAction(buttons[i].name);
        a->setIcon (this->style()->standardIcon(buttons[i].icon));
        connect(a, SIGNAL(triggered()), Api, buttons[i].slot);
    }
}

QToolBar * PlaybackButtons::constructorToolbar(QWidget *parent, DBApi *Api) {
    PlaybackButtons *pbuttons = new PlaybackButtons(parent, Api);
    return pbuttons;
}
