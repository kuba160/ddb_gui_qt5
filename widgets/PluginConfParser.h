#ifndef PLUGINCONFPARSER_H
#define PLUGINCONFPARSER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <dbapi/DBApi.h>

struct PluginConfData {
    QString internalName;
    int instance;
    QString style;
};

class PluginConfParser : public QObject
{
    Q_OBJECT
    DBApi *api = nullptr;
    QJsonDocument config;
public:
    explicit PluginConfParser(QObject *parent, DBApi *Api);

    int getAvailableInstance(QString internalName) const;
    int getTotalInstances(QString internalName) const;

    static QJsonObject pluginObject(QString internalName, int instance, QString style);


    bool insertPlugin(QString internalName, int instance, QString style);
    bool deletePlugin(QString internalName, int instance);
    bool replacePluginStyle(QString internalName, int instance, QString style);
    QList<PluginConfData> getConfiguration();

signals:
};

#endif // PLUGINCONFPARSER_H
