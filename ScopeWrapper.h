#ifndef SCOPEWRAPPER_H
#define SCOPEWRAPPER_H

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

    void process(const ddb_audio_data_t *data);

public slots:
    void setSeries(int num, QAbstractSeries *s);
    void setPaused(bool pause);

signals:
    void fragmentDurationChanged();
    void scopeModeChanged();
    void seriesWidthChanged();
    void pausedChanged();
private:
    void *scope;
    QList<QXYSeries *> series;

    QVector<QPointF> *waveform_data = nullptr;
    size_t waveform_data_size;
    bool reformat_x = false;
    int channels_last = 0;
    int m_paused;

};

#endif // SCOPEWRAPPER_H
