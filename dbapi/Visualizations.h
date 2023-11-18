#ifndef VISUALIZATIONS_H
#define VISUALIZATIONS_H

#include <QObject>
#include <deadbeef/deadbeef.h>

#include "VisScope.h"

class Visualizations : public QObject
{
    Q_OBJECT
    DB_functions_t *deadbeef = nullptr;
public:
    explicit Visualizations(QObject *parent, DB_functions_t *api);

    // internal scope list to dispatch waveforms
    QList<QObject *> scope_list;

public slots:
    //
    QObject *createVisScope(QObject *parent);

protected:
    void onVisScopeDestroyed(QObject * scope);
    static void waveform_callback(void *ctx, const ddb_audio_data_t *data);
};

#endif // VISUALIZATIONS_H
