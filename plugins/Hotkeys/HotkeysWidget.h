#ifndef HOTKEYSWIDGET_H
#define HOTKEYSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QBoxLayout>

#include "HotkeysTreeWidget.h"

class HotkeysWidget : public QWidget
{
    Q_OBJECT
public:
    HotkeysWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Window);
private:
    void createConnections();
    
    QVBoxLayout vbox;
    HotkeysTreeWidget hotkeysTreeWidget;
    QHBoxLayout hbox;
    HotkeyLineEdit hotkeyLineEdit;
    QPushButton clearHotkeyButton;

private Q_SLOTS:
    void catchHotkey();
    void setHotkeyLineEdit();
    void clearHotkey();
    void applyHotkey(const QString &hotkey);

};

#endif // HOTKEYSWIDGET_H
