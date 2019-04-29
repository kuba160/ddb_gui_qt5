#include "HotkeysTreeWidget.h"

#include <QDebug>
#include <QMouseEvent>
#include <QHeaderView>

HotkeysTreeWidget::HotkeysTreeWidget(QWidget* parent): QTreeWidget(parent) {
    QStringList labels;
    labels.append(tr("Action"));
    labels.append(tr("Hotkey"));
    setHeaderLabels(labels);
    header()->setSectionResizeMode(QHeaderView::Stretch);
}

HotkeysTreeWidget::~HotkeysTreeWidget() {
}

void HotkeysTreeWidget::loadHotkeys(DB_plugin_s **plugins) {
    for (int i = 0; plugins[i]; i++) {
        DB_plugin_t *p = plugins[i];
        if (p->get_actions) {
            DB_plugin_action_t *actions = p->get_actions(NULL);
            QTreeWidgetItem *pluginItem;
            if (actions) {
                pluginItem = new QTreeWidgetItem(this);
                pluginItem->setText(0, QString::fromUtf8(p->name));
            }
            while (actions) {
                if (actions->name && actions->title) {
                    QTreeWidgetItem *hotkeyItem = new QTreeWidgetItem(pluginItem);
                    QString title = QString::fromUtf8(actions->title).replace("\\", "");
                    hotkeyItem->setText(0, title);
                    hotkeyItem->setText(2, actions->name);
                    setColumnHidden(2, true);

                    DB_conf_item_t *item = DBAPI->conf_find("hotkeys.", NULL);
                    while (item) {
                        QStringList hotkey = QString::fromUtf8(item->value).split(": ");

                        if (hotkey.length() < 2) {
                            qDebug() << "hotkeys: bad config option " << item->key << " " << item->value;
                            continue;
                        }

                        if (hotkey[1] == QString::fromUtf8(actions->name)) {
                            hotkeyItem->setText(1, hotkey[0]);
                            hotkeys.insert(QString::fromUtf8(item->key), title);
                        }
                        item = DBAPI->conf_find("hotkeys.", item);
                    }

                }
                else {
                    qDebug() << "WARNING: action " << actions->name << "/" << actions->title << " from plugin " << p->name << "is missing name and/or title";
                }
                actions = actions->next;
            }
            if (pluginItem->childCount() == 0)
                delete pluginItem;
        }
    }
}

void HotkeysTreeWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    if (currentItem()->childCount() == 0)
        emit hotkeyDoubleClicked();
    QTreeView::mouseDoubleClickEvent(event);
}

HotkeyLineEdit::HotkeyLineEdit(QWidget *parent): QLineEdit(parent) {
}

HotkeyLineEdit::~HotkeyLineEdit() {
}

void HotkeyLineEdit::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QLineEdit::mousePressEvent(event);
}
