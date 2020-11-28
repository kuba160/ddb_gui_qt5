#include "InterfacePreferencesWidget.h"
#include "ui_InterfacePreferencesWidget.h"

#include "QtGuiSettings.h"
#include "QtGui.h"
#include "GuiUpdater.h"
#undef DBAPI
#define DBAPI deadbeef_internal
InterfacePreferencesWidget::InterfacePreferencesWidget(QWidget *parent, Qt::WindowFlags f):
        QWidget(parent, f),
        ui(new Ui::InterfacePreferencesWidget) {
            
    ui->setupUi(this);
    loadSettings();
    createConnections();
}

void InterfacePreferencesWidget::loadSettings() {
    bool trayIconIsHidden = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::TrayIconIsHidden, false).toBool();
    bool minimizeOnClose = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::MinimizeOnClose, false).toBool();
    bool showTrayTips = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::ShowTrayTips, false).toBool();
    int refreshRate = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::RefreshRate, 10).toInt();
    QString titlebarPlaying = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarPlaying, "%a - %t - DeaDBeeF-%V").toString();
    QString titlebarStopped = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarStopped, "DeaDBeeF-%V").toString();
    QString messageFormat = SETTINGS->getValue(QtGuiSettings::TrayIcon, QtGuiSettings::MessageFormat, "%a - %t").toString();
    
    ui->minimizeCheckBox->setChecked(minimizeOnClose);
    ui->hideTrayCheckBox->setChecked(trayIconIsHidden);
    ui->switchTrackInfoCheckBox->setChecked(showTrayTips);
    ui->trayIconMsgFormatLineEdit->setText(messageFormat);
    ui->refreshRateSlider->setValue(refreshRate);
    ui->titlebarPlayingLineEdit->setText(titlebarPlaying);
    ui->titlebarStoppedLineEdit->setText(titlebarStopped);
    
    ui->switchTrackInfoCheckBox->setVisible(!trayIconIsHidden);
    ui->trayIconMessageLabel->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    ui->trayIconMsgFormatLineEdit->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    
    DBAPI->conf_lock();
    const char **names = DBAPI->plug_get_gui_names();
    for (int i = 0; names[i]; i++) {
        ui->guiPluginComboBox->addItem(QString::fromUtf8(names[i]));
        if (!strcmp(names[i], DBAPI->conf_get_str_fast("gui_plugin", "Qt"))) {
            ui->guiPluginComboBox->setCurrentIndex(i);
        }
    }
    DBAPI->conf_unlock();
}

void InterfacePreferencesWidget::createConnections() {
    connect(ui->hideTrayCheckBox, SIGNAL(toggled(bool)), SLOT(saveTrayIconHidden(bool)));
    connect(ui->switchTrackInfoCheckBox, SIGNAL(toggled(bool)), SLOT(saveTrackInfoOnSwitch(bool)));
    connect(ui->minimizeCheckBox, SIGNAL(toggled(bool)), SLOT(saveCloseOnMinimize(bool)));
    connect(ui->titlebarPlayingLineEdit, SIGNAL(editingFinished()), SLOT(saveTitlePlaying()));
    connect(ui->titlebarStoppedLineEdit, SIGNAL(editingFinished()), SLOT(saveTitleStopped()));
    connect(ui->trayIconMsgFormatLineEdit, SIGNAL(editingFinished()), SLOT(saveTrayMessageFormat()));
    connect(ui->refreshRateSlider, SIGNAL(valueChanged(int)), SLOT(saveRefreshRate(int)));
    connect(ui->guiPluginComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(saveGuiPlugin(QString)));
}

void InterfacePreferencesWidget::saveTrayIconHidden(bool hidden) {
    SETTINGS->setValue(QtGuiSettings::TrayIcon, QtGuiSettings::TrayIconIsHidden, hidden);
    ui->switchTrackInfoCheckBox->setVisible(!hidden);
    ui->trayIconMessageLabel->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    ui->trayIconMsgFormatLineEdit->setVisible(ui->switchTrackInfoCheckBox->isChecked() && ui->switchTrackInfoCheckBox->isVisible());
    emit setTrayIconHidden(hidden);
}

void InterfacePreferencesWidget::saveTrackInfoOnSwitch(bool show) {
    SETTINGS->setValue(QtGuiSettings::TrayIcon, QtGuiSettings::ShowTrayTips, show);
    ui->trayIconMessageLabel->setVisible(show);
    ui->trayIconMsgFormatLineEdit->setVisible(show);
}

void InterfacePreferencesWidget::saveCloseOnMinimize(bool close) {
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::MinimizeOnClose, close);
    emit setCloseOnMinimize(close);
}

void InterfacePreferencesWidget::saveTitlePlaying() {
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarPlaying, ui->titlebarPlayingLineEdit->text());
    emit titlePlayingChanged();
}

void InterfacePreferencesWidget::saveTitleStopped() {
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::TitlebarStopped, ui->titlebarStoppedLineEdit->text());
    emit titleStoppedChanged();
}

void InterfacePreferencesWidget::saveTrayMessageFormat() {
    SETTINGS->setValue(QtGuiSettings::TrayIcon, QtGuiSettings::MessageFormat, ui->trayIconMsgFormatLineEdit->text());
}

void InterfacePreferencesWidget::saveRefreshRate(int refreshRate) {
    SETTINGS->setValue(QtGuiSettings::MainWindow, QtGuiSettings::RefreshRate, refreshRate);
    GuiUpdater::Instance()->resetTimer(refreshRate);
    emit refreshRateChanged();
}

void InterfacePreferencesWidget::saveGuiPlugin(const QString &plugin) {
    DBAPI->conf_set_str("gui_plugin", plugin.toUtf8().constData());
}

void InterfacePreferencesWidget::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
