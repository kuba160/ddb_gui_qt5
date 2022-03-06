#include "ScopeWrapper.h"
#include "scope/scope.h"

#define SCOPE static_cast<ddb_scope_t*>(scope)

ScopeWrapper::ScopeWrapper(QObject *parent) : QObject(parent) {
    scope = ddb_scope_alloc();
    ddb_scope_init(SCOPE);

    //qRegisterMetaType<ScopeWrapper>("ScopeWrapper");
}

ScopeWrapper::~ScopeWrapper() {
    ddb_scope_free(SCOPE);
}


int ScopeWrapper::getFragmentDuration() {
    return SCOPE->fragment_duration;
}

void ScopeWrapper::setFragmentDuration(int dur) {
    if (dur != SCOPE->fragment_duration) {
        SCOPE->fragment_duration = dur;
        emit fragmentDurationChanged();
    }
    return;
}

int ScopeWrapper::getScopeMode() {
    return SCOPE->mode;
}

void ScopeWrapper::setScopeMode(int scope_mode) {
    if (SCOPE->mode != scope_mode) {
        switch (scope_mode) {
            case DDB_SCOPE_MONO:
            case DDB_SCOPE_MULTICHANNEL:
                SCOPE->mode = static_cast<ddb_scope_mode_t>(scope_mode);
                emit scopeModeChanged();
        }
    }
}

int ScopeWrapper::getSamplerate() {
    return SCOPE->samplerate;
}

float *ScopeWrapper::process(int samplerate, int channels, const float *samples, int sample_count) {
    if (SCOPE->samplerate != samplerate) {
        emit samplerateChanged();
    }
    ddb_scope_process(SCOPE, samplerate, channels, samples, sample_count);
    return SCOPE->samples;
}
