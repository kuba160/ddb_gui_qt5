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

typedef struct button_s{
    QString name;
    QStyle::StandardPixmap icon;
    void (PlaybackControl::*slot)();
    //const char *slot;
} button_t;

button_t buttons[] = {{QString("Stop") , QStyle::SP_MediaStop,  &PlaybackControl::stop},
                      {QString("Play") , QStyle::SP_MediaPlay,  &PlaybackControl::play},
                      {QString("Pause"), QStyle::SP_MediaPause, &PlaybackControl::pause},
                      {QString("Previous"),  QStyle::SP_MediaSkipBackward, &PlaybackControl::prev},
                      {QString("Next"),  QStyle::SP_MediaSkipForward, &PlaybackControl::next},
                      {QString(),        QStyle::SP_TitleBarMenuButton, 0}};

PlaybackButtons::PlaybackButtons(QWidget *parent, DBApi *Api) : QWidget(parent) {
    QToolBar *toolbar = qobject_cast<QToolBar *>(parent);
    // Setup
    //this->setIconSize(QSize(16, 16));
    //this->setStyleSheet("QToolButton{padding: 6px;} QToolBar{padding: 0px; margin: 0px;}");

    // Add actions
    int i;
    for (i = 0; buttons[i].slot; i++) {
        //addAction()
        QAction *a = toolbar->addAction(tr(buttons[i].name.toUtf8()));
        a->setIcon (this->style()->standardIcon(buttons[i].icon));
        connect(a, &QAction::triggered, &Api->playback, buttons[i].slot);
        //action_list.append(a);
    }
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);


}


QObject * PlaybackButtons::constructor(QWidget *parent, DBApi *Api) {
    if (!parent) {
        QObject *info = new QObject(nullptr);
        info->setProperty("friendlyName", info->tr("Playback Buttons"));
        info->setProperty("internalName", "playbackButtons");
        info->setProperty("widgetType", "toolbar");
        info->setProperty("widgetStyle", "Qt Widgets");
        return info;
    }
    return new PlaybackButtons(parent, Api);
}

