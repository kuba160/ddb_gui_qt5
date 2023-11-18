#ifndef VISSCOPE_H
#define VISSCOPE_H

#include <deadbeef/deadbeef.h>
#include <QXYSeries>

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
using namespace QtCharts;
#endif

class VisScope : public QObject {
    Q_OBJECT

public:
    VisScope(QObject *parent = nullptr);
    ~VisScope();

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

    Q_PROPERTY(bool channel_offset READ getChannelOffset WRITE setChannelOffset NOTIFY channelOffsetChanged);
    bool getChannelOffset();

    void process(const ddb_audio_data_t *data);

public slots:
    void setSeries(int num,QAbstractSeries *s);
    void setPaused(bool pause);
    void setScale(int s);
    void setChannelOffset(bool on);

signals:
    void fragmentDurationChanged();
    void scopeModeChanged();
    void seriesWidthChanged();
    void pausedChanged();
    void scaleChanged();
    void channelOffsetChanged();
private:
    void *scope;
    QList<QXYSeries *> series;

    int m_paused;
    int m_scale;
    bool m_scale_changed;
    bool m_mode_changed;
    bool m_fragment_duration_changed;
    bool m_channel_offset;

    QVector<QPointF> *data_left = nullptr;
    bool waveform_mono_set = false;
    QVector<QPointF> *data_right = nullptr;
    int channels_last = 0;

};

#endif // VISSCOPE_H
