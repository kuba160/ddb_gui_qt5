#ifndef DBACTIONMENU_H
#define DBACTIONMENU_H

#include <QMenu>
#include <QMenuBar>
#include <QObject>

#include <dbapi/DBApi.h>
#include <dbapi/Actions.h>

class DBActionMenu : public QMenu, public ActionsBuilder {
    ActionContext *context;
public:
    DBActionMenu(QWidget *parent, ActionContext *context);
    ~DBActionMenu();

    virtual ActionsBuilder * createSubMenu(QString &title) override;
    virtual void insertAction(DBAction *action) override;
    virtual void insertSeparator() override;
    virtual QVariant returnValue() override;

signals:
    void actionTriggered(QString);
};

class DBActionMenuBar : public QMenuBar, public ActionsBuilder {
    ActionContext *context;
public:
    DBActionMenuBar(QWidget *parent, ActionContext *context);
    virtual ActionsBuilder * createSubMenu(QString &title) override;
    virtual void insertAction(DBAction *action) override;
    virtual void insertSeparator() override;
    virtual QVariant returnValue() override;
public slots:
    void onActionTriggered(QAction *);

};

void registerMenuBuilders(DBApi *api);

#endif // DBACTIONMENU_H
