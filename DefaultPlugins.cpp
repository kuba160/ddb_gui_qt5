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

#include "VolumeSlider.h"
#include "SeekSlider.h"
#include "PlaybackButtons.h"
#include "CoverArtWidget.h"
#include "PlayListWidget.h"
#include "TabBar.h"

DefaultPlugins::DefaultPlugins() {
    widgetLibrary = new std::vector<DBWidgetInfo>();

    volumeSlider.internalName = QString("volumeSlider");
    volumeSlider.friendlyName = QString("Volume Slider");
    volumeSlider.type = DBWidgetInfo::TypeWidgetToolbar;
    volumeSlider.constructor = VolumeSlider::constructor;
    widgetLibrary->push_back(volumeSlider);

    seekSlider.internalName = QString("seekSlider");
    seekSlider.friendlyName = QString("Seekbar");
    seekSlider.type = DBWidgetInfo::TypeToolbar;
    seekSlider.constructorToolbar = SeekSlider::constructorToolbar;
    widgetLibrary->push_back(seekSlider);

    playbackButtons.internalName = QString("playbackButtons");
    playbackButtons.friendlyName = QString("Playback Buttons");
    playbackButtons.type = DBWidgetInfo::TypeToolbar;
    playbackButtons.constructorToolbar = PlaybackButtons::constructorToolbar;
    widgetLibrary->push_back(playbackButtons);

    artworkWidget.internalName = QString("artwork");
    artworkWidget.friendlyName = QString("Coverart");
    artworkWidget.type = DBWidgetInfo::TypeDockable;
    artworkWidget.constructorDockWidget = CoverArtWidget::constructorDockWidget;
    widgetLibrary->push_back(artworkWidget);

    playlistWidget.internalName = QString("playlist");
    playlistWidget.friendlyName = QString("Playlist");
    playlistWidget.type = DBWidgetInfo::TypeDockable;
    playlistWidget.constructorDockWidget = PlayListWidget::constructorDockable;
    widgetLibrary->push_back(playlistWidget);

    tabBar.internalName = QString("tabBar");
    tabBar.friendlyName = QString("Tab Bar");
    tabBar.type = DBWidgetInfo::TypeWidgetToolbar;
    tabBar.constructor = TabBar::constructor;
    widgetLibrary->push_back(tabBar);

    tabBarD.internalName = QString("tabBarDock");
    tabBarD.friendlyName = QString("Tab Bar (Dock)");
    tabBarD.type = DBWidgetInfo::TypeDockable;
    tabBarD.constructorDockWidget = TabBar::constructorDockable;
    widgetLibrary->push_back(tabBarD);


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
