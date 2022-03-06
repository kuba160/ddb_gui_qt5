#ifndef SCOPEWRAPPER_H
#define SCOPEWRAPPER_H

#include <QWidget>

class ScopeWrapper : public QObject {
    Q_OBJECT

public:
    ScopeWrapper(QObject *parent = nullptr);
    ~ScopeWrapper();


    Q_PROPERTY(int fragment_duration READ getFragmentDuration WRITE setFragmentDuration NOTIFY fragmentDurationChanged);
    int getFragmentDuration();
    void setFragmentDuration(int fragment_duration);

    Q_PROPERTY(int scope_mode READ getScopeMode WRITE setScopeMode NOTIFY scopeModeChanged);
    int getScopeMode();
    void setScopeMode(int scope_mode);

    Q_PROPERTY(int samplerate READ getSamplerate NOTIFY samplerateChanged)
    int getSamplerate();

    float *process(int samplerate, int channels, const float *samples, int sample_count);

signals:
    void fragmentDurationChanged();
    void scopeModeChanged();
    void samplerateChanged();
private:
    void *scope;

};

#endif // SCOPEWRAPPER_H
