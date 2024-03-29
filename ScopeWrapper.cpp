#include <QDebug>
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
    m_scale = 1;
    m_scale_changed = false;
    m_mode_changed = false;
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
        m_fragment_duration_changed = true;
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
                m_mode_changed = true;
                emit scopeModeChanged();
        }
    }
}

int ScopeWrapper::getSeriesWidth() {
    return ((float)SCOPE->fragment_duration)/1000.0 * SCOPE->samplerate / channels_last / m_scale-1;
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
    return m_scale;
}

void ScopeWrapper::setScale(int s) {
    if (s != m_scale) {
        m_scale = s;
        m_scale_changed = true;
        emit scaleChanged();
        emit seriesWidthChanged();
    }
}

void ScopeWrapper::process(const ddb_audio_data_t *data) {
    if (m_paused) {
        return;
    }

    int reformat_x = false;
    if (SCOPE->samplerate != data->fmt->samplerate) {
        emit seriesWidthChanged();
        SCOPE->samplerate = data->fmt->samplerate;
        reformat_x = true;
    }
    if (channels_last != data->fmt->channels) {
        if (!reformat_x) {
            emit seriesWidthChanged();
            reformat_x = true;
        }
        channels_last =  data->fmt->channels;
    }
    if (!data_left || m_scale_changed || m_mode_changed || m_fragment_duration_changed) {
        reformat_x = true;
    }

    size_t total_samples = ((float)SCOPE->fragment_duration)/1000.0 * SCOPE->samplerate;
    bool is_stereo = !(SCOPE->mode == DDB_SCOPE_MONO || data->fmt->channels == 1);

    if (reformat_x) {
        if (!data_left) {
            data_left = new QVector<QPointF>();
        }
        size_t left_size = total_samples/m_scale;
        if (is_stereo) {
            left_size /=2;
        }
        data_left->resize(left_size);
        data_left->squeeze();
        if (!data_right) {
            data_right = new QVector<QPointF>();
        }
        data_right->resize(total_samples/m_scale/2);
        data_right->squeeze();
    }

    QPointF *ptr_l = data_left->data();
    QPointF *ptr_r = data_right->data();

    if (reformat_x) {
        qDebug() << "reformat_x";
        if (is_stereo) {
            for (size_t i = 0; i < total_samples/2/m_scale; i++) {
                ptr_l[i].setX(i);
                ptr_r[i].setX(i);
            }
        }
        else {
            for (size_t i = 0; i < total_samples/m_scale; i++) {
                ptr_l[i].setX(i);
            }
        }
        m_scale_changed = false;
        m_mode_changed = false;
        m_fragment_duration_changed = false;
    }

    ddb_scope_process(SCOPE, data->fmt->samplerate, data->fmt->channels, data->data, data->nframes);

    if (is_stereo) {
        size_t i_out = 0;
        size_t bail_out_at = ((int) total_samples/m_scale/2)*2*m_scale;
        for (size_t i = 0; i < bail_out_at; i+=2*m_scale) {
            for (size_t right_chn = 0; right_chn < 2; right_chn++) {
                float offset = right_chn ? -0.5 : 0.5;
                float value = SCOPE->samples[i+right_chn]*0.5 + offset;
                if (!right_chn) {
                    data_left->replace(i_out,QPointF(i_out,value));
                    //ptr_l[i_out].setY(value);
                }
                else {
                    data_right->replace(i_out,QPointF(i_out,value));
                    //ptr_r[i_out].setY(value);
                }
            }
            i_out++;
        }
        if (series.at(0)) {
            QVector<QPointF> vec(*data_left);
            vec.detach();
            series.at(0)->replace(vec.toList());
        }
        if (series.at(1)) {
            if (!series.at(1)->isVisible()) {
                series.at(1)->setVisible();
            }
            QVector<QPointF> vec(*data_right);
            vec.detach();
            series.at(1)->replace(vec.toList());
        }
    }
    else {
        if (data->fmt->channels == 1) {
            int i_out = 0;
            for (size_t i = 0; i < total_samples; i+=m_scale) {
                ptr_l[i_out].setY(SCOPE->samples[i]);
                i_out++;
            }
        }
        else {
            // merge
            int i_out = 0;
            for (size_t i = 0; i < total_samples; i+=2*m_scale) {
                ptr_l[i_out].setY((SCOPE->samples[i] + SCOPE->samples[i+1])/2);
                i_out++;
            }
        }

        if (series.at(0)) {
            series.at(0)->replace(*data_left);
        }
        if (series.at(1) && series.at(1)->isVisible()) {
            series.at(1)->setVisible(false);
        }
    }
}
