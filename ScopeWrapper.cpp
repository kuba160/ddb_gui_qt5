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
    scale = 1;
    //emit seriesWidthChanged();
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
    return ((float)SCOPE->fragment_duration)/1000.0 * SCOPE->samplerate / channels_last / scale;
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

int ScopeWrapper::getScale() {
    return scale;
}

void ScopeWrapper::setScale(int s) {
    if (s != scale) {
        scale = s;
        emit scaleChanged();
        emit seriesWidthChanged();
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

    size_t size = ((float)SCOPE->fragment_duration)/1000.0 * SCOPE->samplerate / scale;
    bool is_stereo = !(SCOPE->mode == DDB_SCOPE_MONO || data->fmt->channels == 1);
    if (!waveform_data) {
        waveform_data = new QVector<QPointF>(size*scale/2);
        waveform_data->reserve(size*scale);
        reformat_x = true;
    }
    if (!waveform_data2) {
        waveform_data2 = new QVector<QPointF>(size*scale/2);
        waveform_data2->reserve(size*scale/2);
        reformat_x = true;
    }


    if (size != waveform_data_size/2) {
        if (is_stereo) {
            waveform_data->resize(size/2);
            waveform_data2->resize(size/2);
        }
        else {
            waveform_data->resize(size);
        }
        reformat_x = true;
    }

    QPointF *ptr = waveform_data->data();
    QPointF *ptr2 = waveform_data2->data();

    if (reformat_x) {
        if (is_stereo) {
            for (size_t i = 0; i < size/2; i++) {
                ptr[i].setX(i);
                ptr2[i].setX(i);
            }
        }
        else {
            for (size_t i = 0; i < size; i++) {
                ptr[i].setX(i);
            }
        }
        reformat_x = false;
    }

    ddb_scope_process(SCOPE, data->fmt->samplerate, data->fmt->channels, data->data, data->nframes);

    if (is_stereo) {
        bool second_chn = 0;
        for (size_t i = 0; i < size*scale; i++) {
            second_chn = i%2;
            float offset = second_chn ? -0.5 : 0.5;
            float value = SCOPE->samples[i]*0.5 + offset;
            second_chn ? ptr2[i/2/scale].setY(value) : ptr[i/2/scale].setY(value);

            if (second_chn && scale != 1) {
                i += scale;
            }
        }

        if (series.at(0)) {
            series.at(0)->replace(*waveform_data);
        }
        if (series.at(1)) {
            series.at(1)->setVisible();
            series.at(1)->replace(*waveform_data2);
        }
    }
    else {
        if (data->fmt->channels == 1) {
            for (size_t i = 0; i < size*scale; i+=scale) {
                ptr[i/scale].setY(SCOPE->samples[i]);
            }
        }
        else {
            // merge
            for (size_t i = 0; i < size*scale; i+=2*scale) {
                ptr[i/scale].setY((SCOPE->samples[i] + SCOPE->samples[i+1])/2);
            }
        }

        if (series.at(0)) {
            series.at(0)->replace(*waveform_data);
        }
        if (series.at(1)) {
            series.at(1)->setVisible(false);
        }
    }
}
