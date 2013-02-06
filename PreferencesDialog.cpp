#include "PreferencesDialog.h"

#include <include/callbacks.h>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
        QDialog(parent, Qt::WindowTitleHint),
        vbox(this),
        tabWidget(this),
        buttonBox(this),
        interfaceWidget(this),
        soundWidget(this),
        networkWidget(this),
        #ifdef HOTKEYS_ENABLED
        hotkeysWidget(this),
        #endif
        pluginsWidget(this) {

    setModal(true);
    setWindowTitle(tr("Preferences"));
    setWindowIcon(getStockIcon(this, "preferences-system", QStyle::SP_CustomBase));
    buttonBox.setStandardButtons(QDialogButtonBox::Ok);
    configureTabs();
    configureLayout();
    configureConnections();
}

void PreferencesDialog::configureTabs() {
    tabWidget.addTab(&interfaceWidget, tr("Interface"));
    tabWidget.addTab(&soundWidget, tr("Sound"));
    tabWidget.addTab(&networkWidget, tr("Network"));
    tabWidget.addTab(&pluginsWidget, tr("Plugins"));
#ifdef HOTKEYS_ENABLED
    tabWidget.addTab(&hotkeysWidget, tr("Hotkeys"));
#endif
}

void PreferencesDialog::configureLayout() {
    vbox.addWidget(&tabWidget);
    vbox.addWidget(&buttonBox);
}

void PreferencesDialog::configureConnections() {
    connect(&interfaceWidget, SIGNAL(setCloseOnMinimize(bool)), SIGNAL(setCloseOnMinimize(bool)));
    connect(&interfaceWidget, SIGNAL(setTrayIconHidden(bool)), SIGNAL(setTrayIconHidden(bool)));
    connect(&interfaceWidget, SIGNAL(titlePlayingChanged()), SIGNAL(titlePlayingChanged()));
    connect(&interfaceWidget, SIGNAL(titleStoppedChanged()), SIGNAL(titleStoppedChanged()));
    connect(&buttonBox, SIGNAL(accepted()), SLOT(on_buttonBox_accepted()));
}

void PreferencesDialog::on_buttonBox_accepted() {
    DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
    close();
}
