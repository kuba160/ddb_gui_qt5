#ifndef ACTIONSPEC_H
#define ACTIONSPEC_H

#include <QStringList>
#include <QHash>


struct ActionSpec {
    QStringList path;
    QString id;
    uint16_t loc;
    uint16_t arg;
    QHash<QString,QVariant> props;
};

#endif // ACTIONSPEC_H
