#ifndef HOTKEYSTREEWIDGET_H
#define HOTKEYSTREEWIDGET_H

#include "QtGui.h"

#include <QTreeWidget>
#include <QLineEdit>

class HotkeysTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    HotkeysTreeWidget(QWidget *parent = 0);
    ~HotkeysTreeWidget();
    void loadHotkeys(DB_plugin_s **plugins);
    QHash<QString, QString> hotkeys;
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
Q_SIGNALS:
    void hotkeyDoubleClicked();
};

class HotkeyLineEdit : public QLineEdit {
    Q_OBJECT
public:
    HotkeyLineEdit(QWidget* parent = 0);
    ~HotkeyLineEdit();
protected:
    void mousePressEvent(QMouseEvent *event);
Q_SIGNALS:
    void clicked();
};

#endif // HOTKEYSTREEWIDGET_H
