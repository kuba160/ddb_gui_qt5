#include "GuiUpdater.h"

#include <QtGuiSettings.h>

GuiUpdater *GuiUpdater::instance;

GuiUpdater::GuiUpdater(QObject *parent) {
    startSpecificTimer();
}

void GuiUpdater::Destroy() {
    delete instance;
    instance = NULL;
}

GuiUpdater *GuiUpdater::Instance() {
    if (instance == NULL) {
        instance = new GuiUpdater();
    }
    
    return instance;
}

void GuiUpdater::startSpecificTimer(int newTimerTick) {
    if (newTimerTick == -1) {
        int value = SETTINGS->getValue(QtGuiSettings::MainWindow, QtGuiSettings::RefreshRate, 10).toInt();
        startTimer(1000 / value);
    } else {
        startTimer(newTimerTick);
    }
    killTimerAtNextTick = false;
}


void GuiUpdater::resetTimer(int newTimerTick) {
    killTimerAtNextTick = true;
    timerTick = 1000 / newTimerTick;
}

void GuiUpdater::timerEvent(QTimerEvent *event) {
    if (killTimerAtNextTick) {
        killTimer(event->timerId());
        startSpecificTimer(timerTick);
    }
    emit GuiUpdater::Instance()->frameUpdate();
}
