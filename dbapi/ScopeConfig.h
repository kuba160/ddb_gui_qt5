#ifndef SCOPECONFIG_H
#define SCOPECONFIG_H

#include <QObject>
#include <QBindable>
#include <QMutex>
#include <deadbeef/deadbeef.h>
#include <scope/scope.h>

class ScopeConfig : public QObject {
    Q_OBJECT

public:
    ScopeConfig(QObject *parent = nullptr);
    ~ScopeConfig();

    Q_PROPERTY(int fragment_duration READ getFragmentDuration WRITE setFragmentDuration NOTIFY fragmentDurationChanged);
    int getFragmentDuration();
    void setFragmentDuration(int fragment_duration);

    // on mono: only left channgel is visible
    Q_PROPERTY(int scope_mode READ getScopeMode WRITE setScopeMode NOTIFY scopeModeChanged);
    int getScopeMode();
    void setScopeMode(int scope_mode);


    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged);
    bool isPaused();

    Q_PROPERTY(float scale READ getScale WRITE setScale NOTIFY scaleChanged);
    float getScale();

    Q_PROPERTY(int draw_data_points READ getDrawDataPoints WRITE setDrawDataPoints NOTIFY drawDataPointsChanged BINDABLE bindableDrawDataPoints);
    int getDrawDataPoints();

    QMutex draw_data_mut;

public slots:
    void setPaused(bool pause);
    void setScale(float s);
    void setDrawDataPoints(int point_count);
    QBindable<int> bindableDrawDataPoints();


    void process_draw_data(const ddb_audio_data_t *data, ddb_scope_draw_data_t *draw_data);

signals:
    void fragmentDurationChanged();
    void scopeModeChanged();
    void seriesWidthChanged();
    void pausedChanged();
    void scaleChanged();
    void drawDataPointsChanged();

    void dataChanged();
private:
    ddb_scope_t *scope;

    int m_paused;
    float m_scale;

    Q_OBJECT_BINDABLE_PROPERTY(ScopeConfig, int, m_draw_data_points, &ScopeConfig::drawDataPointsChanged)

};

#endif // SCOPECONFIG_H
