#include "Visualizations.h"
#include <QDebug>
#include <QMetaMethod>

Visualizations::Visualizations(QObject *parent, DB_functions_t *api)
    : QObject{parent} {
    deadbeef = api;
}

void Visualizations::scope_callback_attach(QObject *obj, QString method_signature) {

    QByteArray normalizedSignature = QMetaObject::normalizedSignature(method_signature.toUtf8().constData());
    int methodIndex = obj->metaObject()->indexOfMethod(normalizedSignature);
    wf_methods.insert(obj, methodIndex);
    connect(obj, &QObject::destroyed, this, [=]() {
        wf_methods.remove(obj);
    });
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

    QHashIterator<QObject *, int> i(viz->wf_methods);
    while (i.hasNext()) {
        i.next();
        QMetaMethod f = i.key()->metaObject()->method(i.value());
        f.invoke(i.key(),
                 Qt::DirectConnection,
                 Q_ARG(const ddb_audio_data_t*, data));
    }


    for (int i = 0; i < viz->scope_list.length(); i++) {
        VisScope *vs = static_cast<VisScope*>(viz->scope_list.at(i));
        vs->process(data);
    }
}
