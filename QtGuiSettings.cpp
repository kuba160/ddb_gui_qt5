#include "QtGuiSettings.h"
#include "QtGui.h"
#include <QtDebug>
#include "DBApi.h"

QtGuiSettings *settings;

QtGuiSettings::QtGuiSettings(QObject *parent) : QSettings(parent) {
    // dummy
}

QVariant QtGuiSettings::getValue(const QString &group, const QString &key, const QVariant &defaultValue) {
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

void QtGuiSettings::setValue(const QString &group, const QString &key, const QVariant &value) {
    beginGroup(group);
    QSettings::setValue(key, value);
    endGroup();
}

void QtGuiSettings::removeValue(const QString &group, const QString &key) {
    beginGroup(group);
    QSettings::remove(key);
    endGroup();
}
