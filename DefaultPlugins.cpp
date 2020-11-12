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

class Dummy : public QWidget, public DBWidget {
public:
    static QWidget *constructor (QWidget *parent, DBApi *api);
};

QWidget *Dummy::constructor (QWidget *parent, DBApi *api) {
    Q_UNUSED(api);
    return new QWidget(parent);
}

DefaultPlugins::DefaultPlugins() {
    widgetLibrary = new std::vector<DBWidgetInfo>();

    volumeSlider.internalName = QString("volumeSlider");
    volumeSlider.friendlyName = QString("Volume bar");
    volumeSlider.type = DBWidgetInfo::TypeToolbar;
    volumeSlider.constructor = VolumeSlider::constructor;
    widgetLibrary->push_back(volumeSlider);

    seekSlider.internalName = QString("seekSlider");
    seekSlider.friendlyName = QString("Seekbar");
    seekSlider.type = DBWidgetInfo::TypeToolbar;
    seekSlider.constructor = SeekSlider::constructor;
    widgetLibrary->push_back(seekSlider);

    playbackButtons.internalName = QString("playbackButtons");
    playbackButtons.friendlyName = QString("Playback controls");
    playbackButtons.type = DBWidgetInfo::TypeToolbar;
    playbackButtons.constructor = PlaybackButtons::constructor;
    widgetLibrary->push_back(playbackButtons);

    artworkWidget.internalName = QString("artwork");
    artworkWidget.friendlyName = QString("Album Art");
    artworkWidget.type = DBWidgetInfo::TypeMainWidget;
    artworkWidget.constructor = CoverArt::constructor;
    widgetLibrary->push_back(artworkWidget);

    playlistWidget.internalName = QString("playlist");
    playlistWidget.friendlyName = QString("Playlist");
    playlistWidget.type = DBWidgetInfo::TypeMainWidget;
    playlistWidget.constructor = Playlist::constructor;
    widgetLibrary->push_back(playlistWidget);

    tabBar.internalName = QString("tabBar");
    tabBar.friendlyName = QString("Tab strip");
    tabBar.type = DBWidgetInfo::TypeToolbar;
    tabBar.constructor = TabBar::constructor;
    widgetLibrary->push_back(tabBar);

    tabBarD.internalName = QString("tabBarDock");
    tabBarD.friendlyName = QString("%1 (Dock)") .arg(_("Tab strip"));
    tabBarD.type = DBWidgetInfo::TypeMainWidget;
    tabBarD.constructor = TabBar::constructor;
    widgetLibrary->push_back(tabBarD);

    playlistBrowser.internalName = QString("playlistBrowser");
    playlistBrowser.friendlyName = QString("Playlist browser");
    playlistBrowser.type = DBWidgetInfo::TypeMainWidget;
    playlistBrowser.constructor = PlaylistBrowser::constructor;
    widgetLibrary->push_back(playlistBrowser);

    dummy.internalName = QString("dummy");
    dummy.friendlyName = QString("Dummy");
    dummy.type = DBWidgetInfo::TypeMainWidget;
    dummy.constructor = Dummy::constructor;
    widgetLibrary->push_back(dummy);

    medialib.internalName = QString("medialib");
    medialib.friendlyName = QString("Medialib");
    medialib.type = DBWidgetInfo::TypeMainWidget;
    medialib.constructor = Medialib::constructor;
    widgetLibrary->push_back(medialib);
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
