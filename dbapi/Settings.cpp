#include "Settings.h"
#include "models/PluginModel.h"
#include <QDebug>
#include <QFile>

#define DBAPI (this->deadbeef)

Settings::Settings(QObject *parent, DB_functions_t *Api) :
    QSettings(QString("%1/%2.conf") .arg(Api->get_system_dir(DDB_SYS_DIR_CONFIG), "qt5"), QSettings::IniFormat, parent) {
    deadbeef = Api;
    qDebug() << fileName();

    m_plugins = new PluginModel(this, Api);
}


QVariant Settings::get(const QString &group, const QString &key, const QVariant &defaultValue) {
    beginGroup(group);
    QVariant result;
    if (contains(key)) {
        result = value(key, defaultValue);
    }
    else {
        // save default value keys
        QSettings::setValue(key, defaultValue);
        result = defaultValue;
    }
    endGroup();
    return result;
}


void Settings::set(const QString &group, const QString &key, const QVariant &value) {
    beginGroup(group);
    QSettings::setValue(key, value);
    endGroup();
}

void Settings::remove(const QString &group, const QString &key) {
    if (key.isEmpty()) {
        if (group.length() >= 2) {
            QSettings::remove(group);
        }
    }
    beginGroup(group);
    QSettings::remove(key);
    endGroup();
}

QAbstractItemModel* Settings::getPlugins() {
    return m_plugins;
}

QString Settings::getAboutText() {
    QFile aboutDBFile(QString::fromUtf8(DBAPI->get_system_dir(DDB_SYS_DIR_DOC)) + "/about.txt");

    if (aboutDBFile.open(QFile::ReadOnly)) {
        QTextStream s(&aboutDBFile);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        s.setCodec("UTF-8");
#endif
        return s.readAll();
    }
    else {
        return tr("Unable to read file with about information");
    }
}
