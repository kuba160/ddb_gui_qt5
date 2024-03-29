#ifndef SCOPEWRAPPER_H
#define SCOPEWRAPPER_H

#include "scope/scope.h"
#include <deadbeef/deadbeef.h>
#include <QWidget>
#include <QXYSeries>

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
using namespace QtCharts;
#endif

class ScopeWrapper : public QObject {
    Q_OBJECT

public:
    ScopeWrapper(QObject *parent = nullptr);
    ~ScopeWrapper();

    Q_PROPERTY(int fragment_duration READ getFragmentDuration WRITE setFragmentDuration NOTIFY fragmentDurationChanged);
    int getFragmentDuration();
    void setFragmentDuration(int fragment_duration);

    // on mono: only left channgel is visible
    Q_PROPERTY(int scope_mode READ getScopeMode WRITE setScopeMode NOTIFY scopeModeChanged);
    int getScopeMode();
    void setScopeMode(int scope_mode);

    Q_PROPERTY(int series_width READ getSeriesWidth NOTIFY seriesWidthChanged);
    int getSeriesWidth();

    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged);
    bool isPaused();

    Q_PROPERTY(int scale READ getScale WRITE setScale NOTIFY scaleChanged);
    int getScale();

    void process(const ddb_audio_data_t *data);

public slots:
    void setSeries(int num, QtCharts::QAbstractSeries *s);
    void setPaused(bool pause);
    void setScale(int s);

signals:
    void fragmentDurationChanged();
    void scopeModeChanged();
    void seriesWidthChanged();
    void pausedChanged();
    void scaleChanged();
private:
    void *scope;
    QList<QXYSeries *> series;

    int m_paused;
    int m_scale;
    bool m_scale_changed;
    bool m_mode_changed;
    bool m_fragment_duration_changed;

    QVector<QPointF> *data_left = nullptr;
    bool waveform_mono_set = false;
    QVector<QPointF> *data_right = nullptr;
    int channels_last = 0;

};

#endif // SCOPEWRAPPER_H
