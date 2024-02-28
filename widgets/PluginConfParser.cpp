#include "PluginConfParser.h"


PluginConfParser::PluginConfParser(QObject *parent, DBApi *Api) : QObject{parent} {
    api = Api;

    QString c = api->conf.get("DDBW", "plugin_config",
    "[{\"instance\":0,\"internalName\":\"seekSlider\",\"style\":\"Qt Widgets\"},"
    "{\"instance\":0,\"internalName\":\"playlist\",\"style\":\"Qt Widgets\"},"
    "{\"instance\":0,\"internalName\":\"playbackButtons\",\"style\":\"Qt Widgets\"},"
    "{\"instance\":0,\"internalName\":\"volumeSlider\",\"style\":\"Qt Widgets\"},"
    "{\"instance\":0,\"internalName\":\"tabBar\",\"style\":\"Qt Widgets\"}]").toString();
    config = QJsonDocument::fromJson(c.toUtf8(). constData());

    qDebug() << "Json loaded: valid:" << !config.isNull() << config.toJson(QJsonDocument::Compact);
    //api->conf.set("DDBW", "plugin_config", config.toJson(QJsonDocument::Compact));

}


QJsonObject PluginConfParser::pluginObject(QString internalName, int instance, QString style) {
    QJsonObject obj;
    obj.insert("internalName", internalName);
    obj.insert("instance", instance);
    obj.insert("style", style);
    return obj;
}

int PluginConfParser::getAvailableInstance(QString internalName) const {
    QSet<int> instances_taken;
    for (QJsonValue v : config.array()) {
        Q_ASSERT(v.type() == QJsonValue::Object);
        QJsonObject obj = v.toObject();
        Q_ASSERT(obj.contains("internalName") && obj.contains("instance"));
        if (obj.value("internalName").toString() != internalName) {
            continue;
        }
        instances_taken.insert(obj.value("instance").toInt());
    }
    int i = 0;
    while (instances_taken.contains(i)) {
        i++;
    }
    return i;
}

int PluginConfParser::getTotalInstances(QString internalName) const {
    QSet<int> instances_taken;
    for (QJsonValue v : config.array()) {
        Q_ASSERT(v.type() == QJsonValue::Object);
        QJsonObject obj = v.toObject();
        Q_ASSERT(obj.contains("internalName") && obj.contains("instance"));
        if (obj.value("internalName").toString() != internalName) {
            continue;
        }
        instances_taken.insert(obj.value("instance").toInt());
    }
    return instances_taken.count();
}

bool PluginConfParser::insertPlugin(QString internalName, int instance, QString style) {
    // check if exists in config
    for (QJsonValue v : config.array()) {
        Q_ASSERT(v.type() == QJsonValue::Object);
        QJsonObject obj = v.toObject();
        Q_ASSERT(obj.contains("internalName") && obj.contains("instance"));
        if (obj.value("internalName").toString() == internalName &&
            obj.value("instance").toInt() == instance) {
            return false;
        }
    }
    QJsonArray array_new = config.array();
    array_new.append(pluginObject(internalName, instance, style));
    config.setArray(array_new);
    qDebug() << config.toJson();

    api->conf.set("DDBW", "plugin_config", config.toJson(QJsonDocument::Compact));
    return true;
}

bool PluginConfParser::deletePlugin(QString internalName, int instance) {
    bool removed = false;
    for (int i = 0; i < config.array().count(); i++) {
        QJsonValue v = config.array().at(i);
        Q_ASSERT(v.type() == QJsonValue::Object);
        QJsonObject obj = v.toObject();
        Q_ASSERT(obj.contains("internalName") && obj.contains("instance"));
        if (obj.value("internalName").toString() == internalName &&
            obj.value("instance").toInt() == instance) {
            QJsonArray array_new = config.array();
            array_new.removeAt(i);
            config.setArray(array_new);
            removed = true;
            api->conf.set("DDBW", "plugin_config", config.toJson(QJsonDocument::Compact));
            break;
        }
    }
    return removed;
}
bool PluginConfParser::replacePluginStyle(QString internalName, int instance, QString style) {
    bool replaced = false;
    for (int i = 0; i < config.array().count(); i++) {
        QJsonValue v = config.array().at(i);
        Q_ASSERT(v.type() == QJsonValue::Object);
        QJsonObject obj = v.toObject();
        Q_ASSERT(obj.contains("internalName") && obj.contains("instance"));
        if (obj.value("internalName").toString() == internalName &&
            obj.value("instance").toInt() == instance) {
            config.array().replace(i, pluginObject(internalName, instance, style));
            replaced = true;
            api->conf.set("DDBW", "plugin_config", config.toJson(QJsonDocument::Compact));
            break;
        }
    }
    return replaced;
}


QList<PluginConfData> PluginConfParser::getConfiguration() {
    QList<PluginConfData> l;
    for (QJsonValue v : config.array()) {
        Q_ASSERT(v.type() == QJsonValue::Object);
        QJsonObject obj = v.toObject();
        Q_ASSERT(obj.contains("internalName") && obj.contains("instance") && obj.contains("style"));
        l.append({obj.value("internalName").toString(), obj.value("instance").toInt(), obj.value("style").toString()});
    }
    return l;
}
