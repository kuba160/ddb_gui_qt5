#include "SoundPreferencesWidget.h"
#include "ui_SoundPreferencesWidget.h"

#include "QtGui.h"
#undef DBAPI
#define DBAPI deadbeef_internal

static void enumSoundcardCallback(const char *name, const char *desc, void *userdata) {
    SoundPreferencesWidget *dialog = (SoundPreferencesWidget *) (userdata);
    dialog->addDevice(name, desc);
}

SoundPreferencesWidget::SoundPreferencesWidget(QWidget* parent, Qt::WindowFlags f):
        QWidget(parent, f),
        ui(new Ui::SoundPreferencesWidget) {

    ui->setupUi(this);
    loadSettings();
    createConnections();
}

void SoundPreferencesWidget::loadSettings() {
    DBAPI->conf_lock();
    QString s = QString::fromUtf8(DBAPI->conf_get_str_fast("alsa_soundcard", "default"));
    const char *outplugname = DBAPI->conf_get_str_fast("output_plugin", "ALSA output plugin");
    ui->addToPlaylistLineEdit->setText(DBAPI->conf_get_str_fast("cli_add_playlist_name", "Default"));
    ui->replaygainModeComboBox->setCurrentIndex(DBAPI->conf_get_int("replaygain_mode", 0));
    ui->peakScaleCheckBox->setChecked(DBAPI->conf_get_int("replaygain_scale", 1));
    ui->preampSlider->setValue(DBAPI->conf_get_int("replaygain_preamp", 0));
    ui->dontAddFromArchCheckBox->setChecked(DBAPI->conf_get_int("ignore_archives", 1));
    int active = DBAPI->conf_get_int("cli_add_to_specific_playlist", 1);
    ui->addToPlaylistCheckBox->setChecked(active);
    ui->addToPlaylistLineEdit->setEnabled(active);
    ui->resumeOnStartupCheckBox->setChecked(DBAPI->conf_get_int("resume_last_session", 0));
    DBAPI->conf_unlock();
    
    alsaDevices.insert("default", "Default Audio Device");
    
    if (DBAPI->get_output()->enum_soundcards) {
        DBAPI->get_output()->enum_soundcards(enumSoundcardCallback, this);
        ui->outputDeviceComboBox->setEnabled(true);
    }
    else
        ui->outputDeviceComboBox->setEnabled(false);
    
    foreach (QString device, alsaDevices.values()) {
        ui->outputDeviceComboBox->addItem(device);
        if (s == alsaDevices.key(device))
            ui->outputDeviceComboBox->setCurrentIndex(ui->outputDeviceComboBox->count() - 1);
    }
    
    DB_output_t **out_plugs = DBAPI->plug_get_output_list();
    for (int i = 0; out_plugs[i]; i++) {
        ui->outputPluginComboBox->addItem(QString::fromUtf8(out_plugs[i]->plugin.name));
        if (!strcmp(outplugname, out_plugs[i]->plugin.name))
            ui->outputPluginComboBox->setCurrentIndex(i);
    }
    
    if (ui->replaygainModeComboBox->currentIndex() == 0) {
        ui->peakScaleCheckBox->setVisible(false);
        ui->preampLabel->setVisible(false);
        ui->preampSlider->setVisible(false);
        ui->preampValueLabel->setVisible(false);
    }
}

void SoundPreferencesWidget::addDevice(const char *name, const char *desc) {
    alsaDevices.insert(QString::fromUtf8(name), QString::fromUtf8(desc));
}

void SoundPreferencesWidget::createConnections() {
    connect(ui->outputDeviceComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeOutputDevice(int)));
    connect(ui->outputPluginComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeOutputPlugin(int)));
    connect(ui->replaygainModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(changeReplaygainMode(int)));
    connect(ui->peakScaleCheckBox, SIGNAL(toggled(bool)), SLOT(saveReplaygainScale(bool)));
    connect(ui->preampSlider, SIGNAL(sliderReleased()), SLOT(saveReplaygainPreamp()));
    connect(ui->addToPlaylistCheckBox, SIGNAL(toggled(bool)), SLOT(saveAddToDefaultPlaylist(bool)));
    connect(ui->addToPlaylistLineEdit, SIGNAL(editingFinished()), SLOT(saveDefaultPlaylistName()));
    connect(ui->dontAddFromArchCheckBox, SIGNAL(toggled(bool)), SLOT(saveDontAddArchives(bool)));
    connect(ui->resumeOnStartupCheckBox, SIGNAL(toggled(bool)), SLOT(saveResumeOnStartup(bool)));
}

void SoundPreferencesWidget::changeOutputDevice(int deviceNum) {
    DBAPI->conf_set_str("alsa_soundcard", alsaDevices.key(ui->outputDeviceComboBox->currentText()).toUtf8().constData());
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::changeOutputPlugin(int pluginNum) {
    DB_output_t **out_plugs = DBAPI->plug_get_output_list();
    DBAPI->conf_set_str("output_plugin", out_plugs[pluginNum]->plugin.name);
    DBAPI->sendmessage(DB_EV_REINIT_SOUND, 0, 0, 0);
}

void SoundPreferencesWidget::changeReplaygainMode(int index) {
    ui->peakScaleCheckBox->setVisible(index > 0);
    ui->preampLabel->setVisible(index > 0);
    ui->preampSlider->setVisible(index > 0);
    ui->preampValueLabel->setVisible(index > 0);
    DBAPI->conf_set_int("replaygain_mode", index);
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::saveReplaygainScale(bool enabled) {
    DBAPI->conf_set_int("replaygain_scale", enabled);
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::saveReplaygainPreamp() {
    DBAPI->conf_set_float("replaygain_preamp", ui->preampSlider->value());
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
}

void SoundPreferencesWidget::saveAddToDefaultPlaylist(bool enabled) {
    DBAPI->conf_set_int("cli_add_to_specific_playlist", enabled);
    ui->addToPlaylistLineEdit->setEnabled(enabled);
}

void SoundPreferencesWidget::saveDefaultPlaylistName() {
    DBAPI->conf_set_str("cli_add_playlist_name", ui->addToPlaylistLineEdit->text().toUtf8().constData());
}

void SoundPreferencesWidget::saveDontAddArchives(bool enabled) {
    DBAPI->conf_set_int("ignore_archives", enabled);
}

void SoundPreferencesWidget::saveResumeOnStartup(bool enabled) {
    DBAPI->conf_set_int("resume_last_session", enabled);
}

void SoundPreferencesWidget::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
