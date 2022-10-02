#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QAbstractItemModel>
#include <deadbeef/deadbeef.h>


class Settings : public QSettings
{
    Q_OBJECT
    DB_functions_t *deadbeef;
public:
    explicit Settings(QObject *parent, DB_functions_t *Api);

    Q_PROPERTY(QAbstractItemModel* plugins READ getPlugins CONSTANT)

public slots:
    QVariant get(const QString &group, const QString &key, const QVariant &defaultValue);
    void set(const QString &group, const QString &key, const QVariant &value);

    void remove(const QString &group, const QString &key = {});

    QAbstractItemModel* getPlugins();
signals:

private:
    QAbstractItemModel* m_plugins;

};

#endif // SETTINGS_H
