/*
    DefaultPlugins - List of integrated widgets
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
#include "DefaultPlugins.h"

#include <vector>

#include "widgets/VolumeSlider.h"
#include "widgets/SeekSlider.h"
#include "widgets/PlaybackButtons.h"
#include "widgets/Playlist.h"
#include "widgets/PlaylistBrowser.h"
#include "widgets/TabBar.h"
#include "widgets/CoverArt.h"
#include "widgets/Medialib.h"
#include "widgets/LogViewer.h"
#include "widgets/QueueManager.h"
#include "widgets/StatusBar.h"

#undef _
#include "DeadbeefTranslator.h"

class Dummy : public QWidget, public DBWidget {
public:
    static QWidget *constructor (QWidget *parent, DBApi *api);
};

QWidget *Dummy::constructor (QWidget *parent, DBApi *api) {
    Q_UNUSED(api);
    QWidget *widget = new QWidget(parent);
    widget->resize(0,0);
    widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    widget->setVisible(false);
    return widget;
}

DefaultPlugins::DefaultPlugins(QObject *parent) : QObject(parent) {
    widgetLibrary = new std::vector<DBWidgetInfo>();

    volumeSlider.internalName = QString("volumeSlider");
    volumeSlider.friendlyName = QString(tr("Volume bar"));
    volumeSlider.type = DBWidgetInfo::TypeToolbar;
    volumeSlider.constructor = VolumeSlider::constructor;
    volumeSlider.isQuickWidget = false;
    widgetLibrary->push_back(volumeSlider);

    seekSlider.internalName = QString("seekSlider");
    seekSlider.friendlyName = QString(tr("Seekbar"));
    seekSlider.type = DBWidgetInfo::TypeToolbar;
    seekSlider.constructor = SeekSlider::constructor;
    seekSlider.isQuickWidget = false;
    widgetLibrary->push_back(seekSlider);

    playbackButtons.internalName = QString("playbackButtons");
    playbackButtons.friendlyName = QString(tr("Playback controls"));
    playbackButtons.type = DBWidgetInfo::TypeToolbar;
    playbackButtons.constructor = PlaybackButtons::constructor;
    playbackButtons.isQuickWidget = false;
    widgetLibrary->push_back(playbackButtons);

    artworkWidget.internalName = QString("artwork");
    artworkWidget.friendlyName = QString(tr("Album Art"));
    artworkWidget.type = DBWidgetInfo::TypeMainWidget;
    artworkWidget.constructor = CoverArt::constructor;
    artworkWidget.isQuickWidget = false;
    widgetLibrary->push_back(artworkWidget);

    playlistWidget.internalName = QString("playlist");
    playlistWidget.friendlyName = QString(tr("Playlist"));
    playlistWidget.type = DBWidgetInfo::TypeMainWidget;
    playlistWidget.constructor = Playlist::constructor;
    playlistWidget.isQuickWidget = false;
    widgetLibrary->push_back(playlistWidget);

    tabBar.internalName = QString("tabBar");
    tabBar.friendlyName = QString(tr("Tab strip"));
    tabBar.type = DBWidgetInfo::TypeToolbar;
    tabBar.constructor = TabBar::constructor;
    tabBar.isQuickWidget = false;
    widgetLibrary->push_back(tabBar);

    tabBarD.internalName = QString("tabBarDock");
    tabBarD.friendlyName = QString("%1 (Dock)") .arg(tr("Tab strip"));
    tabBarD.type = DBWidgetInfo::TypeMainWidget;
    tabBarD.constructor = TabBar::constructor;
    tabBarD.isQuickWidget = false;
    widgetLibrary->push_back(tabBarD);

    playlistBrowser.internalName = QString("playlistBrowser");
    playlistBrowser.friendlyName = QString(tr("Playlist browser"));
    playlistBrowser.type = DBWidgetInfo::TypeMainWidget;
    playlistBrowser.constructor = PlaylistBrowser::constructor;
    playlistBrowser.isQuickWidget = false;
    widgetLibrary->push_back(playlistBrowser);

    dummy.internalName = QString("dummy");
    dummy.friendlyName = QString("Dummy");
    dummy.type = DBWidgetInfo::TypeMainWidget;
    dummy.constructor = Dummy::constructor;
    dummy.isQuickWidget = false;
    widgetLibrary->push_back(dummy);

    medialib.internalName = QString("medialib");
    medialib.friendlyName = QString("Medialib");
    medialib.type = DBWidgetInfo::TypeMainWidget;
    medialib.constructor = Medialib::constructor;
    medialib.isQuickWidget = false;
    widgetLibrary->push_back(medialib);

    logviewer.internalName = QString("logviewer");
    logviewer.friendlyName = QString(tr("Log"));
    logviewer.type = DBWidgetInfo::TypeMainWidget;
    logviewer.constructor = LogViewer::constructor;
    logviewer.isQuickWidget = false;
    widgetLibrary->push_back(logviewer);

    queueManager.internalName = QString("queuemanager");
    queueManager.friendlyName = QString("Queue Manager");
    queueManager.type = DBWidgetInfo::TypeMainWidget;
    queueManager.constructor = QueueManager::constructor;
    queueManager.isQuickWidget = false;
    widgetLibrary->push_back(queueManager);

    statusBar.internalName = QString("statusbar");
    statusBar.friendlyName = QString(tr("Status Bar"));
    statusBar.type = DBWidgetInfo::TypeStatusBar;
    statusBar.constructor = StatusBar::constructor;
    statusBar.isQuickWidget = false;
    widgetLibrary->push_back(statusBar);

}

DefaultPlugins::~DefaultPlugins() {
    delete widgetLibrary;
}

DBWidgetInfo *DefaultPlugins::WidgetReturn(unsigned long num) {
    if (num>= widgetLibrary->size()) {
        return nullptr;
    }
    return &widgetLibrary->at(num);
};
