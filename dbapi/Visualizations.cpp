#include "Visualizations.h"

#include <QDebug>

Visualizations::Visualizations(QObject *parent, DB_functions_t *api)
    : QObject{parent} {
    deadbeef = api;
}


QObject* Visualizations::createVisScope(QObject *parent) {
    if (!scope_list.length()) {
        deadbeef->vis_waveform_listen(this,waveform_callback);
    }
    VisScope *vs = new VisScope(parent);
    connect(vs, &QObject::destroyed, this, &Visualizations::onVisScopeDestroyed);
    scope_list.append(vs);
    return vs;
}

void Visualizations::onVisScopeDestroyed(QObject *obj) {
    if (scope_list.contains(obj)) {
        scope_list.removeAll(obj);
    }
    if (!scope_list.length()) {
        deadbeef->vis_waveform_unlisten(this);
    }
}

void Visualizations::waveform_callback (void * ctx, const ddb_audio_data_t *data) {
    Visualizations *viz = static_cast<Visualizations *>(ctx);

    for (int i = 0; i < viz->scope_list.length(); i++) {
        VisScope *vs = static_cast<VisScope*>(viz->scope_list.at(i));
        vs->process(data);
    }
}
