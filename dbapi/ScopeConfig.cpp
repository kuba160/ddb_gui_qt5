#include "ScopeConfig.h"
#include <QDebug>
#include <scope/scope.h>

#define SCOPE static_cast<ddb_scope_t*>(scope)

ScopeConfig::ScopeConfig(QObject *parent) : QObject(parent) {
    scope = ddb_scope_alloc();
    ddb_scope_init(SCOPE);

    SCOPE->fragment_duration = 50;
    SCOPE->samplerate = 44100;
    SCOPE->mode = DDB_SCOPE_MULTICHANNEL;
    m_paused = false;
    m_scale = 0.5;

    setDrawDataPoints(100);
}

ScopeConfig::~ScopeConfig() {
    ddb_scope_free(SCOPE);
}

int ScopeConfig::getFragmentDuration() {
    return SCOPE->fragment_duration;
}

void ScopeConfig::setFragmentDuration(int dur) {
    if (dur != SCOPE->fragment_duration) {
        SCOPE->fragment_duration = dur;
        emit fragmentDurationChanged();
        emit seriesWidthChanged();
    }
    return;
}

int ScopeConfig::getScopeMode() {
    return SCOPE->mode;
}

void ScopeConfig::setScopeMode(int scope_mode) {
    if (SCOPE->mode != scope_mode) {
        switch (scope_mode) {
        case DDB_SCOPE_MONO:
        case DDB_SCOPE_MULTICHANNEL:
            SCOPE->mode = static_cast<ddb_scope_mode_t>(scope_mode);
            emit scopeModeChanged();
        }
    }
}

bool ScopeConfig::isPaused() {
    return m_paused;
}

void ScopeConfig::setPaused(bool pause) {
    if (m_paused != pause) {
        m_paused = pause;
        emit pausedChanged();
    }
}

float ScopeConfig::getScale() {
    return m_scale;
}

void ScopeConfig::setScale(float s) {
    if (s != m_scale) {
        m_scale = s;
        emit scaleChanged();
        emit seriesWidthChanged();
    }
}

int ScopeConfig::getDrawDataPoints() {
    return m_draw_data_points;
}

QBindable<int> ScopeConfig::bindableDrawDataPoints() {
    return QBindable<int>(&m_draw_data_points);
}

void ScopeConfig::setDrawDataPoints(int point_count) {
    m_draw_data_points = point_count;
    // m_draw_data_points.set);
    // if (m_draw_data_points != point_count) {
    //     m_draw_data_points = point_count;
    //     emit drawDataPointsChanged();
    // }
}

void ScopeConfig::process_draw_data(const ddb_audio_data_t *data, ddb_scope_draw_data_t *draw_data) {
    // TODO check and maybe fix for mono
    //const int CHANNELS = SCOPE->mode == DDB;
    ddb_scope_process(scope, data->fmt->samplerate, data->fmt->channels, data->data, data->nframes);
    draw_data_mut.lock();
    ddb_scope_get_draw_data (scope, ((float) m_draw_data_points.value()) / ((float) m_scale), 2, 0, draw_data);

    qDebug() << "DRAW POINTS: " << ((float) m_draw_data_points.value()) / ((float) m_scale);
    draw_data_mut.unlock();
}

