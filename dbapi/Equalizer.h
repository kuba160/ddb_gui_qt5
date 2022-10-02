#ifndef EQUALIZER_H
#define EQUALIZER_H

#include <QObject>
#include <deadbeef/deadbeef.h>

class Equalizer : public QObject
{
    Q_OBJECT
    DB_functions_t *deadbeef;
    inline ddb_dsp_context_t * get_supereq();
public:
    explicit Equalizer(QObject *parent, DB_functions_t *api);
    Q_PROPERTY(bool available READ getEqAvailable NOTIFY eqAvailableChanged)
    Q_PROPERTY(bool enabled READ getEqEnabled WRITE setEqEnabled NOTIFY eqEnabledChanged)

    Q_PROPERTY(QVariantList values READ getEq WRITE setEq NOTIFY eqChanged)
    Q_PROPERTY(QVariantList names  READ getEqNames CONSTANT)
signals:
    void eqAvailableChanged();
    void eqEnabledChanged();
    void eqChanged();

public:
    bool getEqAvailable();
    bool getEqEnabled();
    void setEqEnabled(bool);
    QVariantList getEq();
    QVariantList getEqNames();
    void setEq(QVariantList);
};

#endif // EQUALIZER_H
