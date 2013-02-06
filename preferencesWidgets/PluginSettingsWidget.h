#ifndef PLUGINSETTINGSWIDGET_H
#define PLUGINSETTINGSWIDGET_H

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHash>

#include "QtGui.h"

class PluginSettingsWidget : public QGroupBox
{
    Q_OBJECT
public:
    PluginSettingsWidget(ddb_dialog_t *conf, QWidget *parent = 0);
    ~PluginSettingsWidget();

private:
    void configureWidgets();

    ddb_dialog_t *settingsDialog;

    QLabel *label;
    QWidget *prop;
    QHBoxLayout *hbox;
    QPushButton *btn;

    QHash<QWidget *, QString> keys;

public Q_SLOTS:
    void saveProperty();
};

#endif // PLUGINSETTINGSWIDGET_H
