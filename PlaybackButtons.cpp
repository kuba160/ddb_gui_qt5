#include "PlaybackButtons.h"
#include "ui_PlaybackButtons.h"

#include <QStyle>
#include <QIcon>

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
    PlaybackButtons *pbuttons = new PlaybackButtons(parent, Api);
    pbuttons->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    QToolBar *tbar = new QToolBar(parent);
    tbar->addAction(pbuttons->ui->actionNext);
    return tbar;
}
