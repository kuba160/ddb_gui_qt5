#include "Equalizer.h"

#include <QVariant>

#define DBAPI (this->deadbeef)

Equalizer::Equalizer(QObject *parent, DB_functions_t *api)
    : QObject{parent}
{
    deadbeef = api;
    // TODO case DB_EV_DSPCHAINCHANGED:
}

ddb_dsp_context_t * Equalizer::get_supereq() {
    ddb_dsp_context_t *eq_ctx = DBAPI->streamer_get_dsp_chain();
    ddb_dsp_context_t *dsp = eq_ctx;
    while (dsp) {
        if (strcmp(dsp->plugin->plugin.id, "supereq") == 0) {
            return dsp;
        }
        dsp = dsp->next;
    }
    return nullptr;
}

bool Equalizer::getEqAvailable() {
    return get_supereq() ? true : false;
}

bool Equalizer::getEqEnabled() {
    ddb_dsp_context_t *supereq = get_supereq();
    return supereq->enabled;
}

void Equalizer::setEqEnabled(bool enable) {
    if (get_supereq()->enabled != enable) {
        get_supereq()->enabled = enable;
        emit eqEnabledChanged();
    }
}

QVariantList Equalizer::getEq() {
    QList<QVariant> l;
    ddb_dsp_context_t *supereq = get_supereq();
    if (supereq) {
        char buf[255];
        int param_count = supereq->plugin->num_params();
        for (int i = 0; i < param_count; i++) {
            supereq->plugin->get_param(supereq, i, buf, 255);
            l.append(atof(buf));
        }
    }
    else {
        //qDebug() << "getEq(): supereq unavailable!";
    }
    return l;
}

void Equalizer::setEq(QVariantList eq) {
    ddb_dsp_context_t *supereq = get_supereq();
    if (supereq) {
        QVariantList eq_curr = getEq();
        bool changed = false;
        int param_count = supereq->plugin->num_params();
        for (int i = 0; i < param_count; i++) {
            if (eq[i].toFloat() != eq_curr[i]) {
                if (eq[i].toFloat() > 20.0) {
                    eq[i] = 20.0;
                }
                else if (eq[i].toFloat() < -20.0) {
                    eq[i] = -20.0;
                }
                char buf[255];
                snprintf(buf, 255, "%f", eq[i].toFloat());
                supereq->plugin->set_param(supereq, i, buf);
                changed = true;
            }
        }
        if (changed) {
            DBAPI->streamer_dsp_refresh();
            DBAPI->streamer_dsp_chain_save();
            emit eqChanged();
        }
    }
    else {
        //qDebug() << "setEq(): supereq unavailable!";
    }
}

QVariantList Equalizer::getEqNames() {
    QList<QVariant> l;
    ddb_dsp_context_t *supereq = get_supereq();
    if (supereq) {
        int param_count = supereq->plugin->num_params();
        for (int i = 0; i < param_count; i++) {
            l.append(supereq->plugin->get_param_name(i));
        }
    }
    else {
        //qDebug() << "getEq(): supereq unavailable!";
    }
    return l;
}
