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

    Q_PROPERTY(QString aboutText READ getAboutText CONSTANT)
    Q_PROPERTY(QString aboutGPLV2 READ getAboutGPLV2 CONSTANT)
    Q_PROPERTY(QString aboutLGPLV21 READ getAboutLGPLV21 CONSTANT)
    Q_PROPERTY(QString aboutTranslators READ getAboutTranslators CONSTANT)
    Q_PROPERTY(QString aboutChangelog READ getAboutChangelog CONSTANT)



public slots:
    QVariant get(const QString &group, const QString &key, const QVariant &defaultValue);
    void set(const QString &group, const QString &key, const QVariant &value);

    void remove(const QString &group, const QString &key = {});

    QAbstractItemModel* getPlugins();

    QString getAboutText();
    QString getAboutGPLV2();
    QString getAboutLGPLV21();
    QString getAboutTranslators();
    QString getAboutChangelog();

    void ddbSet(QString key, QString value);
    void ddbSet(QString key, int value);
    void ddbSet(QString key, int64_t value);
    void ddbSet(QString key, float value);

signals:
    void settingChanged(QString group, QString key);
    void ddbSettingChanged(QString key);

private:
    QAbstractItemModel* m_plugins;

    QString ddb_read_text_file(QString file);

};

#endif // SETTINGS_H
