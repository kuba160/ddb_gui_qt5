#include "ActionsPlaceholder.h"
#include <QMetaMethod>

DBActionPlaceholder::DBActionPlaceholder(QObject *parent, ActionSpec spec) : DBAction(parent) {
    this->title = spec.path.last();
    this->path = spec.path;
    this->action_id = spec.id;
    this->locations = spec.loc;
    this->accepts = spec.arg;
    this->properties_const = spec.props;
}


bool DBActionPlaceholder::apply(PlayItemIterator &context) {
    emit actionApplied(context);
    return true;
}

QHash<QString,QVariant> DBActionPlaceholder::contextualize(PlayItemIterator &context) const {
    QHash<QString,QVariant> ret;

    static const QMetaMethod actionAppliedSignal = QMetaMethod::fromSignal(&DBAction::actionApplied);
    // assume implementable to true if not specified
    if (properties_const.contains("implementable") && !properties_const.value("implementable").toBool()) {
        return ret;
    }
    else if (!isSignalConnected(actionAppliedSignal)) {
        ret.insert("enabled", false);
        QString t = this->title + " (not implemented)";
        ret.insert("text", t);
    }
    return ret;
}



ActionsPlaceholder::ActionsPlaceholder(QObject *parent)
    : ActionOwner{parent}
{

    const QList<struct ActionSpec> l = {
        ActionSpec {
            .path = QStringList{"File", "Open file(s)"},
            .id = "q_open_files",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("document-open")} }
        },
        ActionSpec {
            .path = QStringList{"File", "Add file(s)"},
            .id = "q_add_files",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("list-add")} }
        },
        ActionSpec {
            .path = QStringList{"File", "Add folders(s)"},
            .id = "q_add_folders",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("folder-open")} }
        },
        ActionSpec {
            .path = QStringList{"File", "Add location"},
            .id = "q_add_location",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("folder-remote")} } // "document-open-remote"
        },
        ActionSpec { // TODO THIS CAN BE IMPLEMENTED
            .path = QStringList{"File", "New playlist"},
            .id = "q_new_playlist",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("media-playlist-append")} }
        },
        ActionSpec {
            .path = QStringList{"File", "Load playlist"},
            .id = "q_load_playlist",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("folder-download")} }
        },
        ActionSpec {
            .path = QStringList{"File", "Save playlist"},
            .id = "q_save_playlist",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("document-save")} }
        },
        ActionSpec {
            .path = QStringList{"File", "Quit"},
            .id = "q_quit",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("application-exit")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Clear"},
            .id = "q_clear",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("edit-clear-list")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Select all"},
            .id = "q_select_all",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("edit-select-all")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Deselect all"},
            .id = "q_deselect_all",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("edit-select-none")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Invert selection"},
            .id = "q_invert_selection",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("edit-select-invert")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Selection", "Remove"},
            .id = "q_selection_remove",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("edit-delete")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Selection", "Crop"},
            .id = "q_selection_crop",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("trim-to-selection")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Find"},
            .id = "q_find",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("edit-find")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Sort by", "Title"},
            .id = "q_sortby_title",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("sort-name")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Sort by", "Track number"},
            .id = "q_sortby_tracknumber",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("view-media-track")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Sort by", "Album"},
            .id = "q_sortby_album",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("view-media-album-cover")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Sort by", "Artist"},
            .id = "q_sortby_artist",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("view-media-artist")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Sort by", "Date"},
            .id = "q_sortby_date",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("view-calendar-date")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Sort by", "Custom"},
            .id = "q_sortby_custom",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("document-edit")} }
        },
        ActionSpec {
            .path = QStringList{"Edit", "Preferences"},
            .id = "q_preferences",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", QVariant("settings-configure")},
                                              {"separator_before", QVariant(true)} }
        },
        ActionSpec {
            .path = QStringList{"View", "Design mode"},
            .id = "q_design_mode",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"enabled", QVariant(false)} }
        },
        ActionSpec {
            .path = QStringList{"Playback", "Scroll follows playback"},
            .id = "q_scroll_follows_playback",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"implementable", QVariant(false)},
                                              {"checkable", QVariant(true)}}
        },
        ActionSpec {
            .path = QStringList{"Playback", "Cursor follows playback"},
            .id = "q_cursor_follows_playback",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"implementable", QVariant(false)},
                                              {"checkable", QVariant(true)}}
        },
//        ActionSpec {
//            .path = QStringList{"Playback", "Stop after current track"},
//            .id = "q_stop_after_current_track",
//            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
//            .arg = DBAction::ACTION_ARG_NONE,
//            .props = QHash<QString,QVariant>{ {"implementable", QVariant(false)},
//                                              {"checkable", QVariant(true)},
//                                              {"config", "playlist.stop_after_current"}}
//        },
//        ActionSpec {
//            .path = QStringList{"Playback", "Stop after current album"},
//            .id = "q_stop_after_current_album",
//            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
//            .arg = DBAction::ACTION_ARG_NONE,
//            .props = QHash<QString,QVariant>{ {"implementable", QVariant(false)},
//                                              {"checkable", QVariant(true)},
//                                              {"config", "playlist.stop_after_album"},
//                                              {"separator_after", QVariant(true)}}
//        },
        ActionSpec {
            .path = QStringList{"Playback", "Jump to current track"},
            .id = "q_jump_to_current_track",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"implementable", QVariant(false)},
                                              {"icon", "go-jump"}}
        },
        // Help
        ActionSpec {
            .path = QStringList{"Help", "Help"},
            .id = "q_help",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", "help-about"} }
        },
        ActionSpec {
            .path = QStringList{"Help", "ChangeLog"},
            .id = "q_changelog",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", "text-x-changelog"},
                                              {"separator-after", QVariant(true)}}
        },
        ActionSpec {
            .path = QStringList{"Help", "GPLv2"},
            .id = "q_gplv2",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", "license"}}
        },
        ActionSpec {
            .path = QStringList{"Help", "LGPLv2.1"},
            .id = "q_lgplv21",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", "license"}}
        },
        ActionSpec {
            .path = QStringList{"Help", "About"},
            .id = "q_about",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", "help-about"},
                                              {"separator-before", true}}
        },
        ActionSpec {
            .path = QStringList{"Help", "About Qt"},
            .id = "q_about_qt",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", "help-about"}}
        },
        ActionSpec {
            .path = QStringList{"Help", "Translators"},
            .id = "q_translators",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"icon", "languages"}}
        },
        // track context
        ActionSpec {
            .path = QStringList{"Cut"},
            .id = "q_cut",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_TRACK_CONTEXT,
            .arg = DBAction::ACTION_ARG_TRACK | DBAction::ACTION_ARG_TRACKS,
            .props = QHash<QString,QVariant>{{"icon", "edit-cut"}}
        },

        ActionSpec {
            .path = QStringList{"Copy"},
            .id = "q_copy",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_TRACK_CONTEXT,
            .arg = DBAction::ACTION_ARG_TRACK | DBAction::ACTION_ARG_TRACKS,
            .props = QHash<QString,QVariant>{{"icon", "edit-copy"}}
        },

        ActionSpec {
            .path = QStringList{"Paste"},
            .id = "q_paste",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_TRACK_CONTEXT,
            .arg = DBAction::ACTION_ARG_TRACK | DBAction::ACTION_ARG_TRACKS,
            .props = QHash<QString,QVariant>{{"icon", "edit-paste"}, {"separator_after", true}}
        },

        ActionSpec {
            .path = QStringList{"Track properties"},
            .id = "q_track_properties",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_TRACK_CONTEXT,
            .arg = DBAction::ACTION_ARG_TRACK | DBAction::ACTION_ARG_TRACKS,
            .props = QHash<QString,QVariant>{{"icon", "document-properties"}, {"separator_before", true}, {"last", true}}
        }
    };

    for (const ActionSpec &spec : l) {
        DBActionPlaceholder *action = new DBActionPlaceholder(this, spec);
        m_actions.append(action);
    }
}

QList<DBAction*> ActionsPlaceholder::getActions() {
    return m_actions;
}
DBAction* ActionsPlaceholder::getAction(QString action_id) {
    for (DBAction *action : m_actions) {
        if (action->action_id == action_id) {
            return action;
        }
    }
    return nullptr;
}
