#ifndef GUIUPDATER_H
#define GUIUPDATER_H

#include <QObject>
#include <QTimerEvent>

class GuiUpdater : public QObject {
    Q_OBJECT
public:
    static GuiUpdater *Instance();
    static void Destroy();

    void resetTimer(int tick);
    
private:
    GuiUpdater(QObject* parent = 0);
    static GuiUpdater *instance;

    bool killTimerAtNextTick;
    void startSpecificTimer(int newTimerTick = -1);
    int timerTick;
    
protected:
    void timerEvent(QTimerEvent *event);
signals:
    void frameUpdate();
};

#endif // GUIUPDATER_H
