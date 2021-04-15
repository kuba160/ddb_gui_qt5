#include "callbacks.h"

#include <QWidget>
#include <QStyle>

#include "QtGui.h"
//#include "qticonloader.h"
#undef DBAPI
#define DBAPI deadbeef_internal

QIcon getStockIcon(QWidget *widget, const QString &freedesktop_name, int fallback) {
    QIcon fallbackIcon;
    if (fallback > 0) {
        fallbackIcon = widget->style()->standardIcon(QStyle::StandardPixmap(fallback), 0, widget);
    }
    return fallbackIcon.isNull() ? QIcon::fromTheme(freedesktop_name, fallbackIcon) : fallbackIcon;
}
