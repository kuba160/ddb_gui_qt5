#include "HotkeyReadDialog.h"
#include <QKeyEvent>
#include <QDebug>
#include "QtGui.h"

HotkeyReadDialog::HotkeyReadDialog(const QString &actionTitle, QWidget *parent):
        QDialog(parent, Qt::Dialog),
        message(this),
        hotkey(this),
        vbox(this) {

    setModal(true);
    setWindowTitle(tr("Hotkey for action \"%1\"").arg(actionTitle));
    message.setText(tr("Press the new keys for action \"%1\"").arg(actionTitle));
    vbox.addWidget(&message);
    vbox.addWidget(&hotkey);
    setLayout(&vbox);
}

HotkeyReadDialog::~HotkeyReadDialog() {

}

void HotkeyReadDialog::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
    case Qt::Key_Shift:
        hotkey.setText(hotkey.text() + "Shift ");
        return;
        break;
    case Qt::Key_Control:
        hotkey.setText(hotkey.text() + "Ctrl ");
        return;
        break;
    case Qt::Key_Meta:
        hotkey.setText(hotkey.text() + "Super ");
        return;
        break;
    case Qt::Key_Alt:
        hotkey.setText(hotkey.text() + "Alt ");
        return;
        break;
    default:
        break;
    }
    const char *name = HOTKEYS->get_name_for_keycode(e->nativeVirtualKey());
    hotkey.setText(hotkey.text() + QString::fromUtf8(name));
}

void HotkeyReadDialog::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Shift ||
            e->key() == Qt::Key_Control ||
            e->key() == Qt::Key_Meta ||
            e->key() == Qt::Key_Alt)
        return;
    emit hotkeyChanged(hotkey.text());
    close();
}
