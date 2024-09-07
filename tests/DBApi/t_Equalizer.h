#ifndef T_EQUALIZER_H
#define T_EQUALIZER_H

#include <QObject>
#include "dbapi/DBApi.h"

class t_Equalizer : public QObject
{
    Q_OBJECT
public:
    explicit t_Equalizer(DBApi *Api);

private:
    DBApi *api;

private slots:

    void eqEnableTest();
    void eqSetTest();

signals:
};

#endif // T_EQUALIZER_H
