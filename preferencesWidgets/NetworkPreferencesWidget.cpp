#include "NetworkPreferencesWidget.h"
#include "ui_NetworkPreferencesWidget.h"

#include "QtGui.h"

#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

NetworkPreferencesWidget::NetworkPreferencesWidget(QWidget* parent, Qt::WindowFlags f):
        QWidget(parent, f),
        ui(new Ui::NetworkPreferencesWidget)
{
    ui->setupUi(this);
    loadSettings();
    createConnections();
}

void NetworkPreferencesWidget::loadSettings() {
    ui->enableProxyCheckBox->setChecked(DBAPI->conf_get_int("network.proxy", 0));
    DBAPI->conf_lock();
    ui->proxyAddressLineEdit->setText(QString::fromUtf8(DBAPI->conf_get_str_fast("network.proxy.address", "")));
    ui->proxyPortSpinBox->setValue(QString::fromUtf8(DBAPI->conf_get_str_fast("network.proxy.port", "8080")).toInt());
    const char *type = DBAPI->conf_get_str_fast("network.proxy.type", "HTTP");
    DBAPI->conf_unlock();
    if (!strcasecmp(type, "HTTP")) {
        ui->proxyTypeComboBox->setCurrentIndex(0);
    }
    else if (!strcasecmp(type, "HTTP_1_0")) {
        ui->proxyTypeComboBox->setCurrentIndex(1);
    }
    else if (!strcasecmp(type, "SOCKS4")) {
        ui->proxyTypeComboBox->setCurrentIndex(2);
    }
    else if (!strcasecmp(type, "SOCKS5")) {
        ui->proxyTypeComboBox->setCurrentIndex(3);
    }
    else if (!strcasecmp(type, "SOCKS4A")) {
        ui->proxyTypeComboBox->setCurrentIndex(4);
    }
    else if (!strcasecmp(type, "SOCKS5_HOSTNAME")) {
        ui->proxyTypeComboBox->setCurrentIndex(5);
    }
    ui->proxyUsernameLineEdit->setText(QString::fromUtf8(DBAPI->conf_get_str_fast("network.proxy.username", "")));
    ui->proxyPasswordLineEdit->setText(QString::fromUtf8(DBAPI->conf_get_str_fast("network.proxy.password", "")));
    
    bool enabled = ui->enableProxyCheckBox->isChecked();
    
    ui->proxyAddressLineEdit->setVisible(enabled);
    ui->proxyPasswordLabel->setVisible(enabled);
    ui->proxyPasswordLineEdit->setVisible(enabled);
    ui->proxyPortLabel->setVisible(enabled);
    ui->proxyPortSpinBox->setVisible(enabled);
    ui->proxyServerLabel->setVisible(enabled);
    ui->proxyServerTypeLabel->setVisible(enabled);
    ui->proxyTypeComboBox->setVisible(enabled);
    ui->proxyUsernameLabel->setVisible(enabled);
    ui->proxyUsernameLineEdit->setVisible(enabled);
}

void NetworkPreferencesWidget::createConnections() {
    connect(ui->enableProxyCheckBox, SIGNAL(toggled(bool)), SLOT(enableProxy(bool)));
    connect(ui->proxyAddressLineEdit, SIGNAL(editingFinished()), SLOT(saveProxyAddress()));
    connect(ui->proxyPasswordLineEdit, SIGNAL(editingFinished()), SLOT(saveProxyPassword()));
    connect(ui->proxyPortSpinBox, SIGNAL(editingFinished()), SLOT(saveProxyPort()));
    connect(ui->proxyTypeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(saveProxyType(int)));
    connect(ui->proxyUsernameLineEdit, SIGNAL(editingFinished()), SLOT(saveProxyUsername()));
}

void NetworkPreferencesWidget::enableProxy(bool enabled) {
    DBAPI->conf_set_int("network.proxy", enabled);
    ui->proxyAddressLineEdit->setVisible(enabled);
    ui->proxyPasswordLabel->setVisible(enabled);
    ui->proxyPasswordLineEdit->setVisible(enabled);
    ui->proxyPortLabel->setVisible(enabled);
    ui->proxyPortSpinBox->setVisible(enabled);
    ui->proxyServerLabel->setVisible(enabled);
    ui->proxyServerTypeLabel->setVisible(enabled);
    ui->proxyTypeComboBox->setVisible(enabled);
    ui->proxyUsernameLabel->setVisible(enabled);
    ui->proxyUsernameLineEdit->setVisible(enabled);
}

void NetworkPreferencesWidget::saveProxyAddress() {
    DBAPI->conf_set_str("network.proxy.address", ui->proxyAddressLineEdit->text().toUtf8().constData());
}

void NetworkPreferencesWidget::saveProxyPassword() {
    DBAPI->conf_set_str("network.proxy.password", ui->proxyPasswordLineEdit->text().toUtf8().constData());
}

void NetworkPreferencesWidget::saveProxyPort() {
    DBAPI->conf_set_int("network.proxy.port", ui->proxyPortSpinBox->value());
}

void NetworkPreferencesWidget::saveProxyType(int index) {
    switch (index) {
        case 0:
            DBAPI->conf_set_str("network.proxy.type", "HTTP");
            break;
        case 1:
            DBAPI->conf_set_str("network.proxy.type", "HTTP_1_0");
            break;
        case 2:
            DBAPI->conf_set_str("network.proxy.type", "SOCKS4");
            break;
        case 3:
            DBAPI->conf_set_str("network.proxy.type", "SOCKS5");
            break;
        case 4:
            DBAPI->conf_set_str("network.proxy.type", "SOCKS4A");
            break;
        case 5:
            DBAPI->conf_set_str("network.proxy.type", "SOCKS5_HOSTNAME");
            break;
        default:
            DBAPI->conf_set_str("network.proxy.type", "HTTP");
            break;
    }
}

void NetworkPreferencesWidget::saveProxyUsername() {
    DBAPI->conf_set_str("network.proxy.username", ui->proxyUsernameLineEdit->text().toUtf8().constData());
}

void NetworkPreferencesWidget::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}
