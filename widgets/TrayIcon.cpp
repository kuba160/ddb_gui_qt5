#include <QIcon>
#include <QDebug>
#include "TrayIcon.h"
#include "DBActionMenu.h"

TrayIcon::TrayIcon(QObject *parent, DBApi *api) : QSystemTrayIcon(parent) {
    setIcon(QIcon(":/images/deadbeef.png"));
    setVisible(true);

    menu = buildTrayContextMenu(nullptr, api);
    setContextMenu(menu);
}

TrayIcon::~TrayIcon() {
    delete menu;
}
