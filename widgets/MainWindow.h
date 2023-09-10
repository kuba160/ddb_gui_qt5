#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../dbapi/DBApi.h"

#include "PluginManager.h"
#include "ActionHandlers.h"

class MainWindow : public QMainWindow
{
    QString title_playing;
    Q_OBJECT
    DBApi *api;
public:
    explicit MainWindow(QWidget *parent, DBApi *Api);
    ~MainWindow();

    void closeEvent(QCloseEvent *ev) override;
    QMenu * createPopupMenu() override;

protected:
    PluginManager plugins;
    ActionHandlers action_handlers;
signals:

};

#endif // MAINWINDOW_H
