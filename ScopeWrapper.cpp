#include "ScopeWrapper.h"
#include "scope/scope.h"

#define SCOPE static_cast<ddb_scope_t*>(scope)

ScopeWrapper::ScopeWrapper(QObject *parent) : QObject(parent) {
    scope = ddb_scope_alloc();
    ddb_scope_init(SCOPE);

    series.append(nullptr);
    series.append(nullptr);
    SCOPE->fragment_duration = 50;
    SCOPE->samplerate = 44100;
    SCOPE->mode = DDB_SCOPE_MULTICHANNEL;
    channels_last = 2;
    m_paused = false;
    emit seriesWidthChanged();
    //qRegisterMetaType<ScopeWrapper>("ScopeWrapper");
}

ScopeWrapper::~ScopeWrapper() {
    ddb_scope_free(SCOPE);
}

void ScopeWrapper::setSeries(int pos, QAbstractSeries *s) {
    if (pos >= 0 && pos <=1) {
        series.replace(pos,static_cast<QXYSeries*>(s));
    }
}

int ScopeWrapper::getFragmentDuration() {
    return SCOPE->fragment_duration;
}

void ScopeWrapper::setFragmentDuration(int dur) {
    if (dur != SCOPE->fragment_duration) {
        SCOPE->fragment_duration = dur;
        emit fragmentDurationChanged();
        emit seriesWidthChanged();
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
                reformat_x = true;
                emit scopeModeChanged();
        }
    }
}

int ScopeWrapper::getSeriesWidth() {
    return ((float)SCOPE->fragment_duration)/1000.0 * SCOPE->samplerate / channels_last;
}

bool ScopeWrapper::isPaused() {
    return m_paused;
}

void ScopeWrapper::setPaused(bool pause) {
    if (m_paused != pause) {
        m_paused = pause;
        emit pausedChanged();
    }
}

void ScopeWrapper::process(const ddb_audio_data_t *data) {
    if (m_paused) {
        return;
    }

    if (SCOPE->samplerate != data->fmt->samplerate) {
        emit seriesWidthChanged();
        SCOPE->samplerate = data->fmt->samplerate;
    }
    if (channels_last != data->fmt->channels) {
        reformat_x = true;
        channels_last= data->fmt->channels;
        emit seriesWidthChanged();
    }

    size_t size = ((float)SCOPE->fragment_duration)/1000.0 * SCOPE->samplerate;
    if (!waveform_data) {
        waveform_data = new QVector<QPointF>(size);
        reformat_x = true;
    }
    else if (size != waveform_data_size) {
        delete waveform_data;
        waveform_data = new QVector<QPointF>(size);
        reformat_x = true;
    }
    if (reformat_x) {
        QPointF *ptr = waveform_data->data();
        if (SCOPE->mode == DDB_SCOPE_MONO || data->fmt->channels == 1) {
            for (size_t i = 0; i < size; i++) {
                ptr[i].setX(i);
            }
        }
        else {
            for (size_t i = 0; i < size; i++) {
                ptr[i].setX(i%(size/2));
            }
        }
        reformat_x = false;
    }

    ddb_scope_process(SCOPE, data->fmt->samplerate, data->fmt->channels, data->data, data->nframes);
    QPointF *ptr = waveform_data->data();

    if (SCOPE->mode == DDB_SCOPE_MONO || data->fmt->channels == 1) {
        if (data->fmt->channels == 1) {
            for (size_t i = 0; i < size; i++) {
                ptr[i].setY(SCOPE->samples[i]);
            }
        }

        if (series.at(0)) {
            series.at(0)->replace(*waveform_data);
        }
        if (series.at(1)) {
            series.at(1)->setVisible(false);
        }
        // merge
        //TODO
    }
    else {
        for (size_t i = 0; i < size; i++) {
            float offset = data->fmt->channels == 1 ? 0 : i%2 ? -0.5 : 0.5;
            int shift = i%2 ? 0 : size/2;
            float value = offset ? SCOPE->samples[i]*0.5 +offset : SCOPE->samples[i];
            ptr[i/2+shift].setY(value);
        }

        if (series.at(0)) {
            series.at(0)->replace(waveform_data->mid(0,size/2));
        }
        if (series.at(1)) {
            series.at(1)->setVisible();
            series.at(1)->replace(waveform_data->mid(size/2,size/2));
        }
    }

    //api->series_list.at(i)->replace(*api->waveform_data);

}
