#ifndef PLUGINSPREFERENCESWIDGET_H
#define PLUGINSPREFERENCESWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QTextEdit>

#include "QtGui.h"
#include "PluginSettingsWidget.h"

class CopyrightDialog : public QDialog {
  Q_OBJECT
public:
    CopyrightDialog(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Window);
    void setText(const QString &);
private:
    QVBoxLayout vbox;
    QTextEdit textEdit;
};

namespace Ui {
    class PluginsPreferencesWidget;
}

class PluginsPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    PluginsPreferencesWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Window);
    ~PluginsPreferencesWidget();
    
private:
    Ui::PluginsPreferencesWidget *ui;

    void createPluginsSettings();
    void configurePluginSettingsPanel(ddb_dialog_t *);
    
    DB_plugin_s **plugins;
    
    QSpacerItem *spacer;
    
    PluginSettingsWidget *settingsWidget;

    CopyrightDialog copyrightDialog;
    
protected:
    void changeEvent(QEvent *e);

private Q_SLOTS:
    void loadPluginInfo(int);
    void openUrl();
};



#endif // PLUGINSPREFERENCESWIDGET_H
