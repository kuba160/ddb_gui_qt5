#ifndef DEFAULTACTIONS_H
#define DEFAULTACTIONS_H

#include <QWidget>
#include <QMenuBar>
#include "DBApi.h"
//#include "ui_DefaultActions.h"

namespace Ui {
class DefaultActions;
}

class DefaultActions : public QWidget, public DBWidget
{
    Q_OBJECT
public:
    explicit DefaultActions(DBApi *Api = nullptr, QWidget *parent = nullptr);
    QMenuBar *getDefaultMenuBar();
private:
    Ui::DefaultActions *ui;

    QActionGroup *repeatGroup;
    QAction *repeat[3];
    QActionGroup *shuffleGroup;
    QAction *shuffle[4];

    QMenu *main_widgets;
    QActionGroup *main_widgets_list;
    QMenu *new_plugins;
    QMenu *remove_plugins;

public slots:
    void shuffleRepeatHandler();
    void onWidgetAddAction(QAction *);
    void onWidgetRemoveAction(QAction *);
    void onMainWidgetAdded(QAction *);
    void onActionToggleCreated(QAction *);

private slots:
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionPreferences_triggered();
    void on_actionExit_triggered();
};

#endif // DEFAULTACTIONS_H
