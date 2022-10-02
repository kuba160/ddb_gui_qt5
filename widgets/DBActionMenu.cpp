#include "DBActionMenu.h"
#include <QDebug>

DBActionMenu::DBActionMenu(QWidget *parent, ActionContext *context) : QMenu(parent) {
    this->context = context;

    connect(this, &QMenu::triggered, this, [this](QAction *action) {
        qDebug() << "HI";
        DBAction* db_action = action->property("DBAction").value<DBAction*>();
        if (db_action) {
            qDebug() << "GOOD";
            this->context->executeForAction(db_action);
        }
    });
}

DBActionMenu::~DBActionMenu() {

}


ActionsBuilder * DBActionMenu::createSubMenu(QString &title) {
    qDebug() << "DBACTIONMENU: new submenu: " << title;
    DBActionMenu *child = new DBActionMenu(this, this->context);
    child->setTitle(title);
    this->addMenu(child);
    return child;

}
void DBActionMenu::insertAction(DBAction *action) {
    qDebug() << "DBACTIONMENU: new action: " << action->title;
    QAction *a = addAction(action->title);
    a->setProperty("DBAction", QVariant::fromValue(action));
    a->setProperty("action_id", action->action_id);
    a->setIcon(QIcon::fromTheme(action->icon));
}

void DBActionMenu::insertSeparator() {
    addSeparator();
}

QVariant DBActionMenu::returnValue() {
    return QVariant::fromValue(this);
}

DBActionMenuBar::DBActionMenuBar(QWidget *parent, ActionContext *context) : QMenuBar(parent) {
    this->context = context;
}

ActionsBuilder * DBActionMenuBar::createSubMenu(QString &title) {
    DBActionMenu *child = new DBActionMenu(this, context);
    child->setTitle(title);
    addMenu(child);
    return child;
}

void DBActionMenuBar::insertAction(DBAction *action) {
    // shouldn't be called??
}

void DBActionMenuBar::insertSeparator() {
    // shouldn't be called
}

QVariant DBActionMenuBar::returnValue() {
    return QVariant::fromValue(this);
}

ActionsBuilder * menubar_constructor(QObject *parent, ActionContext *context) {
    QWidget *parent_widget = qobject_cast<QWidget*>(parent);
    if (parent_widget) {
        return new DBActionMenuBar(parent_widget, context);
    }
    return nullptr;
}

ActionsBuilder * contextmenu_constructor(QObject *parent, ActionContext *context) {
    QWidget *parent_widget = qobject_cast<QWidget*>(parent);
    if (parent_widget) {
        return new DBActionMenu(parent_widget, context);
    }
    return nullptr;
}

void registerMenuBuilders(DBApi *api) {
    api->actions.registerActionsBuilder("widgets_mainmenu", menubar_constructor);
    api->actions.registerActionsBuilder("widgets_contextmenu", contextmenu_constructor);
}
