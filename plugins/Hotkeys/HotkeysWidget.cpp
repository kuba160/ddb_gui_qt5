#include "HotkeysWidget.h"

#include "QtGui.h"
#include "HotkeyReadDialog.h"

HotkeysWidget::HotkeysWidget(QWidget *parent, Qt::WindowFlags f):
        QWidget(parent, f),
        vbox(),
        hotkeysTreeWidget(this),
        hbox(),
        hotkeyLineEdit(this),
        clearHotkeyButton(tr("Clear"), this) {

    hotkeyLineEdit.setAlignment(Qt::AlignHCenter);
    hotkeyLineEdit.setReadOnly(true);
    clearHotkeyButton.setEnabled(false);
    hbox.addWidget(&hotkeyLineEdit);
    hbox.addWidget(&clearHotkeyButton);
    hotkeysTreeWidget.loadHotkeys(DBAPI->plug_get_list());
    vbox.addWidget(&hotkeysTreeWidget);
    vbox.addLayout(&hbox);
    setLayout(&vbox);
    createConnections();
}

void HotkeysWidget::createConnections() {
    connect(&hotkeysTreeWidget, SIGNAL(itemSelectionChanged()), SLOT(setHotkeyLineEdit()));
    connect(&hotkeysTreeWidget, SIGNAL(hotkeyDoubleClicked()), SLOT(catchHotkey()));
    connect(&hotkeyLineEdit, SIGNAL(clicked()), SLOT(catchHotkey()));
    connect(&clearHotkeyButton, SIGNAL(pressed()), SLOT(clearHotkey()));
}

void HotkeysWidget::setHotkeyLineEdit() {
    const QString text = hotkeysTreeWidget.currentItem()->text(1);
    hotkeyLineEdit.setText(text);
    clearHotkeyButton.setEnabled(text != "");
}

void HotkeysWidget::catchHotkey() {
    if (!hotkeysTreeWidget.currentItem() || hotkeysTreeWidget.currentItem()->childCount() != 0)
        return;
    HotkeyReadDialog readHotkeyDialog(hotkeysTreeWidget.currentItem()->text(0), this);
    connect(&readHotkeyDialog, SIGNAL(hotkeyChanged(const QString &)), SLOT(applyHotkey(const QString &)));
    readHotkeyDialog.exec();
}

void HotkeysWidget::applyHotkey(const QString &hotkey) {
    const QString title = hotkeysTreeWidget.currentItem()->text(0);
    const QString name = hotkeysTreeWidget.currentItem()->text(2);
    QString key = hotkeysTreeWidget.hotkeys.key(title);
    if (key == "") {
        key = QString("hotkeys.key%1").arg(hotkeysTreeWidget.hotkeys.size() + 1);
    }
    const QString value = (hotkey != "" ? hotkey : hotkeyLineEdit.text()) + ": " + name;
    DBAPI->conf_set_str(key.toUtf8().constData(), value.toUtf8().constData());
    HOTKEYS->reset();
    hotkeysTreeWidget.hotkeys.insert(key, title);
    hotkeysTreeWidget.currentItem()->setText(1, hotkey);
    setHotkeyLineEdit();
}

void HotkeysWidget::clearHotkey() {
    const QString title = hotkeysTreeWidget.currentItem()->text(0);
    DBAPI->conf_remove_items(hotkeysTreeWidget.hotkeys.key(title).toUtf8().constData());
    HOTKEYS->reset();
    hotkeyLineEdit.clear();
    hotkeysTreeWidget.currentItem()->setText(1, "");
}