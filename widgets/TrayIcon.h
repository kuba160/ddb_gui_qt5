#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include "dbapi/DBApi.h"

class TrayIcon : public QSystemTrayIcon
{
public:
    TrayIcon(QObject *parent = nullptr, DBApi *api = nullptr);
    ~TrayIcon();

protected:
    QMenu *menu;
};

#endif // TRAYICON_H
