
#include "ActionsConfig.h"

#define DBAPI (api->deadbeef)

QList<DBActionShuffle *> action_shuffle_list;

DBActionShuffle::DBActionShuffle(QObject *parent, DBApi *Api, ddb_shuffle_t shuffle): DBAction(parent) {
    api = Api;
    this->shuffle = shuffle;
    this->locations = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR;
    this->accepts = DBAction::ACTION_ARG_NONE;
    this->properties_const = QHash<QString,QVariant>{ {"checkable", QVariant(true)},
                                                      {"exclusive_group", "shuffle_group"}};
    switch(shuffle) {
    case DDB_SHUFFLE_OFF:
        this->path = QStringList{"Playback", "Shuffle", "Off"};
        this->action_id = "q_shuffle_off";
        break;
    case DDB_SHUFFLE_TRACKS:
        this->path = QStringList{"Playback", "Shuffle", "Tracks"};
        this->action_id = "q_shuffle_tracks";
        break;
    case DDB_SHUFFLE_ALBUMS:
        this->path = QStringList{"Playback", "Shuffle", "Albums"};
        this->action_id = "q_shuffle_albums";
        break;
    case DDB_SHUFFLE_RANDOM:
        this->path = QStringList{"Playback", "Shuffle", "Random Tracks"};
        this->action_id = "q_shuffle_random";
        break;
    }
    this->title = this->path.last();

    action_shuffle_list.append(this);
}

DBActionShuffle::~DBActionShuffle() {
    action_shuffle_list.remove(action_shuffle_list.indexOf(this));
}

QHash<QString,QVariant> DBActionShuffle::contextualize(PlayItemIterator &context) const {
    QHash<QString,QVariant> ret;
    ret.insert(this->properties_const);

    ddb_shuffle_t curr = DBAPI->streamer_get_shuffle();
    ret.insert("checked", curr == shuffle);
    return ret;
}

bool DBActionShuffle::apply(PlayItemIterator &context) {
    if (shuffle != DBAPI->streamer_get_shuffle()) {
        api->playback.setShuffle(static_cast<PlaybackControl::shuffle>(shuffle));
        for (DBActionShuffle *a : action_shuffle_list) {
            emit a->actionPropertiesChanged();
        }
        return true;
    }
    return false;
}

QList<DBActionRepeat *> action_repeat_list;

DBActionRepeat::DBActionRepeat(QObject *parent, DBApi *Api, ddb_repeat_t repeat): DBAction(parent) {
    api = Api;
    this->repeat = repeat;
    this->locations = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR;
    this->accepts = DBAction::ACTION_ARG_NONE;
    this->properties_const = QHash<QString,QVariant>{ {"checkable", QVariant(true)},
                                                      {"exclusive_group", "repeat_group"}};
    switch(repeat) {
    case DDB_REPEAT_ALL:
        this->path = QStringList{"Playback", "Repeat", "All Tracks"};
        this->action_id = "q_repeat_all";
        break;
    case DDB_REPEAT_SINGLE:
        this->path = QStringList{"Playback", "Repeat", "One Track"};
        this->action_id = "q_repeat_one";
        break;
    case DDB_REPEAT_OFF:
        this->path = QStringList{"Playback", "Repeat", "Off"};
        this->action_id = "q_repeat_off";
        break;
    }
    this->title = this->path.last();

    action_repeat_list.append(this);
}

DBActionRepeat::~DBActionRepeat() {
    action_repeat_list.remove(action_repeat_list.indexOf(this));
}

QHash<QString,QVariant> DBActionRepeat::contextualize(PlayItemIterator &context) const {
    QHash<QString,QVariant> ret;
    ret.insert(this->properties_const);

    ddb_repeat_t curr = DBAPI->streamer_get_repeat();
    ret.insert("checked", curr == repeat);
    return ret;
}

bool DBActionRepeat::apply(PlayItemIterator &context) {
    if (repeat != DBAPI->streamer_get_repeat()) {
        api->playback.setRepeat(static_cast<PlaybackControl::repeat>(repeat));
        for (DBActionRepeat *a : action_repeat_list) {
            emit a->actionPropertiesChanged();
        }
        return true;
    }
    return false;
}


DBActionConfigBool::DBActionConfigBool(QObject *parent, DBApi *Api, ActionSpec spec) : DBAction(parent) {
    api = Api;
    this->locations = spec.loc;
    this->accepts = spec.arg;
    this->action_id = spec.id;
    this->path = spec.path;
    this->properties_const = spec.props;

    assert (properties_const.contains("config"));
    properties_const.insert("checkable", true);

    config_str = properties_const.value("config").toString();
    config_default = properties_const.value("config_default").toBool();

    current_value = DBAPI->conf_get_int(config_str.toUtf8().constData(),config_default);
}

QHash<QString,QVariant> DBActionConfigBool::contextualize(PlayItemIterator &context) const {
    QHash<QString,QVariant> ret;
    ret.insert(this->properties_const);

    int v = DBAPI->conf_get_int(config_str.toUtf8().constData(), value_def);
    ret.insert("checked", (bool) (v));
    return ret;
}

bool DBActionConfigBool::apply(PlayItemIterator &context) {
    value = !DBAPI->conf_get_int(config_str.toUtf8().constData(), config_default);
    DBAPI->conf_set_int(config_str.toUtf8().constData(), value);
    return true;
}

void DBActionConfigBool::onConfigChanged() {
    int new_value = DBAPI->conf_get_int(config_str.toUtf8().constData(), config_default);
    if (current_value != new_value) {
        current_value = new_value;
        emit actionPropertiesChanged();
    }
}

DBActionPropertyBool::DBActionPropertyBool(QObject *parent, ActionSpec spec) : DBAction(parent) {
    this->locations = spec.loc;
    this->accepts = spec.arg;
    this->action_id = spec.id;
    this->path = spec.path;
    this->properties_const = spec.props;

}

void DBActionPropertyBool::connectProperty(QObject *property_owner, QString prop_name) {
    prop_owner = property_owner;

    const QMetaObject *mo = prop_owner->metaObject();
    int property_idx = mo->indexOfProperty(prop_name.toUtf8().constData());
    if (property_idx == -1) {
        qDebug() << "Failed to create DBActionProperty for" << mo->className() << "and property" << prop_name << "!";
    }
    else {
        prop_meta = mo->property(property_idx);
        if (!prop_meta.hasNotifySignal()) {
            qDebug() << "Property" << prop_name << "for" << mo->className() << "does not have notify signal!";
        }
    }


    QMetaMethod propertyNotifySignal = prop_meta.notifySignal();
    // get DBAction's actionPropertiesChanged signal
    QMetaMethod actionPropertiesChangedSignal =  QMetaMethod::fromSignal(&DBAction::actionPropertiesChanged);
    connect(prop_owner, propertyNotifySignal, static_cast<DBAction*>(this), actionPropertiesChangedSignal);
}

QHash<QString,QVariant> DBActionPropertyBool::contextualize(PlayItemIterator &context) const {
    QHash<QString,QVariant> ret;
    ret.insert(this->properties_const);
    ret.insert("checked", prop_meta.read(prop_owner).toBool());
    return ret;
}

bool DBActionPropertyBool::apply(PlayItemIterator &context) {
    bool current = prop_meta.read(prop_owner).toBool();
    prop_meta.write(prop_owner, !current);
    return true;
}

ActionsConfig::ActionsConfig(QObject *parent, DBApi *Api)
    : ActionOwner{parent}
{

    const QList<ActionSpec> speclist = {
        ActionSpec {
            .path = QStringList{"Playback", "Stop after current track"},
            .id = "q_stop_after_current_track",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"implementable", QVariant(false)},
                                              {"checkable", QVariant(true)},
                                              {"property", "stop_after_current"}}
        },
        ActionSpec {
            .path = QStringList{"Playback", "Stop after current album"},
            .id = "q_stop_after_current_album",
            .loc = DBAction::ACTION_LOC_HOTKEY | DBAction::ACTION_LOC_MENUBAR,
            .arg = DBAction::ACTION_ARG_NONE,
            .props = QHash<QString,QVariant>{ {"implementable", QVariant(false)},
                                              {"checkable", QVariant(true)},
                                              {"property", "stop_after_album"},
                                              {"separator_after", QVariant(true)}}
        }
    };

    for (const ActionSpec &spec : speclist) {
        auto *prop = new DBActionPropertyBool(this, spec);
        m_actions.append(prop);
        prop->connectProperty(&Api->playback, spec.props.value("property").toString());
    }



    const QList<ddb_shuffle_t> shuf_list = {DDB_SHUFFLE_OFF, DDB_SHUFFLE_TRACKS, DDB_SHUFFLE_ALBUMS, DDB_SHUFFLE_RANDOM};
    for (const ddb_shuffle_t shuf : shuf_list) {
        DBActionShuffle *action = new DBActionShuffle(this, Api, shuf);
        m_actions.append(action);
    }


    const QList<ddb_repeat_t> repeat_list = {DDB_REPEAT_ALL, DDB_REPEAT_SINGLE, DDB_REPEAT_OFF};
    for (const ddb_repeat_t repeat : repeat_list) {
        DBActionRepeat *action = new DBActionRepeat(this, Api, repeat);
        m_actions.append(action);
    }

}

ActionsConfig::~ActionsConfig() {
    while(!m_actions.empty()) {
        DBAction *action = m_actions.takeFirst();
        emit actionAboutToBeDeleted(action->action_id);
        delete action;
    }
}


QList<DBAction*> ActionsConfig::getActions() {
    return m_actions;
}
DBAction* ActionsConfig::getAction(QString action_id) {
    for (DBAction *action : m_actions) {
        if (action->action_id == action_id) {
            return action;
        }
    }
    return nullptr;
}
