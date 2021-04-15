#ifndef QTGUISETTINGS_H
#define QTGUISETTINGS_H

#define SETTINGS settings

#include <QVariant>
#include <QString>
#include <QSettings>

#define QSSETCONF(X,Y) autoSetValue(this, X, Y)
#define QSGETCONF(X,Y) autoGetValue(this, X, Y)


class QtGuiSettings : public QSettings {
    Q_OBJECT
public:
    QtGuiSettings(QObject *parent);
    //QtGuiSettings(const QString &fileName, QSettings::Format format, QObject *parent = nullptr);
    //~QtGuiSettings();

    QVariant getValue(const QString &group, const QString &key, const QVariant &defaultValue);
    void setValue(const QString &group, const QString &key, const QVariant &value);

    void removeValue(const QString &group, const QString &key);

private:
    //static QtGuiSettings *instance;

    //QSettings settings;
};

extern QtGuiSettings *settings;

#endif // QTGUISETTINGS_H
