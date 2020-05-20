#include "PlaybackButtons.h"
#include "ui_PlaybackButtons.h"

#include <QStyle>
#include <QIcon>

button_t buttons[] = {{QString("stop") , QStyle::SP_MediaStop,  SLOT(stop())},
                      {QString("play") , QStyle::SP_MediaPlay,  SLOT(play())},
                      {QString("pause"), QStyle::SP_MediaPause, SLOT(togglePause())},
                      {QString("prev"),  QStyle::SP_MediaSkipBackward, SLOT(playPrev())},
                      {QString("next"),  QStyle::SP_MediaSkipForward, SLOT(playNext())},
                      {QString(), QStyle::SP_TitleBarMenuButton, ""}};

PlaybackButtons::PlaybackButtons(QWidget *parent, DBApi *Api) : QWidget(parent),
                                                                DBToolbarWidget(parent, Api),
                                                                ui(new Ui::PlaybackButtons) {
    ui->setupUi(this);
    ui->actionPlay->setIcon  (this->style()->standardIcon(QStyle::SP_MediaPlay));
    ui->actionPause->setIcon (this->style()->standardIcon(QStyle::SP_MediaPause));
    ui->actionStop->setIcon  (this->style()->standardIcon(QStyle::SP_MediaStop));
    ui->actionPrev->setIcon  (this->style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->actionNext->setIcon  (this->style()->standardIcon(QStyle::SP_MediaSkipForward));

    // connections
    connect(ui->actionPlay, SIGNAL(triggered()), Api, SLOT(play()));
    connect(ui->actionNext, SIGNAL(triggered()), Api, SLOT(playNext()));
    connect(ui->actionPrev, SIGNAL(triggered()), Api, SLOT(playPrev()));
    connect(ui->actionPause, SIGNAL(triggered()), Api, SLOT(togglePause()));
    connect(ui->actionStop, SIGNAL(triggered()), Api, SLOT(stop()));



    //
    //this->setIconSize(QSize(16, 16));
    //ui->PlaybackToolbar->setFixedHeight(39);
    //ui->PlaybackToolbar->setStyleSheet("QToolButton{padding: 6px;}");
}

QToolBar * PlaybackButtons::constructorToolbar(QWidget *parent, DBApi *Api) {
/*    PlaybackButtons *pbuttons = new PlaybackButtons(parent, Api);
QStyle::SP_MediaPlay
    QStyle table[] = {QStyle::SP_MediaPlay};

    ->actionPlay->setIcon  (this->style()->standardIcon(QStyle::SP_MediaPlay));
    ui->actionPause->setIcon (this->style()->standardIcon(QStyle::SP_MediaPause));
    ui->actionStop->setIcon  (this->style()->standardIcon(QStyle::SP_MediaStop));
    ui->actionPrev->setIcon  (this->style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->actionNext->setIcon  (this->style()->standardIcon(QStyle::SP_MediaSkipForward));

    // connections
    connect(ui->actionPlay, SIGNAL(triggered()), Api, SLOT(play()));
    connect(ui->actionNext, SIGNAL(triggered()), Api, SLOT(playNext()));
    connect(ui->actionPrev, SIGNAL(triggered()), Api, SLOT(playPrev()));
    connect(ui->actionPause, SIGNAL(triggered()), Api, SLOT(togglePause()));
    connect(ui->actionStop, SIGNAL(triggered()), Api, SLOT(stop()));
    pbuttons->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
*/
    QToolBar *tbar = new QToolBar(parent);
    tbar->setIconSize(QSize(16, 16));
    //tbar->setFixedHeight(39);
    tbar->setStyleSheet("QToolButton{padding: 6px;}");
    defaultButtonsAdd(tbar, Api);
    //tbar->addWidget(pbuttons);
    return tbar;
}

void PlaybackButtons::defaultButtonsAdd(QToolBar *toolbar, DBApi *Api) {
    int i;
    for (i = 0; buttons[i].slot[0]; i++) {
        QAction *a = toolbar->addAction(buttons[i].name);
        a->setIcon (toolbar->style()->standardIcon(buttons[i].icon));
        connect(a, SIGNAL(triggered()), Api, buttons[i].slot);
    }
}
