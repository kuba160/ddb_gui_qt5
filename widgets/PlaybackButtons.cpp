/*
    PlaybackButtons - Playback Buttons Widget
    Copyright (C) 2019-2020 Jakub Wasylków <kuba_160@protonmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "PlaybackButtons.h"

button_t buttons[] = {{QString("Stop") , QStyle::SP_MediaStop,  SLOT(stop())},
                      {QString("Play") , QStyle::SP_MediaPlay,  SLOT(play())},
                      {QString("Pause"), QStyle::SP_MediaPause, SLOT(togglePause())},
                      {QString("Previous"),  QStyle::SP_MediaSkipBackward, SLOT(playPrev())},
                      {QString("Next"),  QStyle::SP_MediaSkipForward, SLOT(playNext())},
                      {QString(),        QStyle::SP_TitleBarMenuButton, 0}};

PlaybackButtons::PlaybackButtons(QWidget *parent, DBApi *Api) : QToolBar(parent), DBWidget(parent, Api) {
    // Setup
    this->setIconSize(QSize(16, 16));
    this->setStyleSheet("QToolButton{padding: 6px;} QToolBar{padding: 0px; margin: 0px;}");

    // Add actions
    int i;
    for (i = 0; buttons[i].slot; i++) {
        QAction *a = this->addAction(tr(buttons[i].name.toUtf8()));
        a->setIcon (this->style()->standardIcon(buttons[i].icon));
        connect(a, SIGNAL(triggered()), Api, buttons[i].slot);
    }

    // Menu initial setup
    parent->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(parent,SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));
    this->setContextMenuPolicy(Qt::PreventContextMenu);
    menu.addAction("Edit playback buttons... (TODO)")->setEnabled(false);

}

QWidget * PlaybackButtons::constructor(QWidget *parent, DBApi *Api) {
    return new PlaybackButtons(parent, Api);
}

void PlaybackButtons::customContextMenuRequested(QPoint pos) {
    if (parent()->property("DesignMode").toBool()) {
        menu.move(parentWidget()->mapToGlobal(pos));
        menu.show();
    }
}
