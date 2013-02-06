#include "StatusBar.h"

#include <QMenu>

#include "QtGui.h"
#include <QtGuiSettings.h>
#include "GuiUpdater.h"

StatusBar::StatusBar(QWidget *parent) : QStatusBar(parent) {
    lastBitrateUpdate = QDateTime::currentDateTime().time();
    addWidget(&statusLabel);
    loadConfig();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showPopUp(QPoint)));
    connect(GuiUpdater::Instance(), SIGNAL(frameUpdate()), this, SLOT(onFrameUpdate()));
}

StatusBar::~StatusBar() {
    saveConfig();
}

void StatusBar::saveConfig() {
    SETTINGS->setValue(QtGuiSettings::StatusBar, QtGuiSettings::PlayingFormat, playingFormat);
    SETTINGS->setValue(QtGuiSettings::StatusBar, QtGuiSettings::PausedFormat, pausedFormat);
    SETTINGS->setValue(QtGuiSettings::StatusBar, QtGuiSettings::StoppedFormat, stoppedFormat);
}

void StatusBar::loadConfig() {
    QString defaultPlayingValue = "%status | %format | %bitrate | %samplerate | %bitspersample | %channels | %curtime / %duration | %tracksnum tracks | %totaltime total playtime";
    QString defaultStoppedValue = "Stopped | %1 tracks | %2 total playtime";
    playingFormat = SETTINGS->getValue(QtGuiSettings::StatusBar, QtGuiSettings::PlayingFormat, defaultPlayingValue).toString();
    pausedFormat = SETTINGS->getValue(QtGuiSettings::StatusBar, QtGuiSettings::PausedFormat, defaultPlayingValue).toString();
    stoppedFormat = SETTINGS->getValue(QtGuiSettings::StatusBar, QtGuiSettings::StoppedFormat, defaultStoppedValue).toString();
}

const QString StatusBar::getTotalTimeString() {
    float totaltime = DBAPI->pl_get_totaltime();

    int daystotal = (int)totaltime / (3600 * 24);

    QString daystotal_s = QString("%1").arg(daystotal).rightJustified(2, '0');
    QString hourtotal = QString("%1").arg(((int)totaltime / 3600) % 24).rightJustified(2, '0');
    QString mintotal = QString("%1").arg(((int)totaltime / 60) % 60).rightJustified(2, '0');
    QString sectotal = QString("%1").arg(((int)totaltime) % 60).rightJustified(2, '0');

    if (daystotal == 0) {
        return QString("%1:%2:%3").arg(hourtotal).arg(mintotal).arg(sectotal);
    } else
        if (daystotal == 1) {
            return QString(tr("1 day %1:%2:%3")).arg(hourtotal).arg(mintotal).arg(sectotal);
        } else {
            return QString(tr("%1 days %2:%3:%4")).arg(daystotal_s).arg(hourtotal).arg(mintotal).arg(sectotal);
        }
    return "";
}

void StatusBar::showPopUp(QPoint point) {
    QMenu *popupMenu = new QMenu(this);

    QAction *action = popupMenu->addAction(tr("Set Format..."));
    connect(action, SIGNAL(triggered()), this, SLOT(setFormat()));

    popupMenu->exec(mapToGlobal(point));
}

void StatusBar::setFormat() {
    //TODO open custom dialog
}

void StatusBar::onFrameUpdate() {
    if (isHidden() || parentWidget()->isHidden())
        return;
    
    QString statusString;

    QString totaltime_str = getTotalTimeString();
    DB_playItem_t *track = DBAPI->streamer_get_playing_track();
    DB_fileinfo_t *c = DBAPI->streamer_get_current_fileinfo();
    float duration = track ? DBAPI->pl_get_item_duration(track) : -1;
    if (DBAPI->get_output()->state() == OUTPUT_STATE_STOPPED || !track || !c) {
        //TODO
        statusString = QString(stoppedFormat).arg(DBAPI->pl_getcount(PL_MAIN)).arg(totaltime_str);
    } else {
        float playpos = DBAPI->streamer_get_playpos();
        int minpos = playpos / 60;
        int secpos = playpos - minpos * 60;
        int mindur = duration / 60;
        int secdur = duration - mindur * 60;

        QString mode;

        if (c->fmt.channels <= 2) {
            mode = QString(c->fmt.channels == 1 ? "Mono" : "Stereo");
        } else {
            mode = QString("%1ch Multichannel").arg(c->fmt.channels);
        }
        int samplerate = c->fmt.samplerate;
        int bitspersample = c->fmt.bps;

        QString t;
        if (duration >= 0) {
            t = QString("%1:%2").arg(mindur).arg(QString("%1").arg(secdur).rightJustified(2, '0'));
        } else {
            t = QString("-:--");
        }

        QTime tm = QDateTime::currentDateTime().time();
        if (tm.second() - lastBitrateUpdate.second() + (tm.msec() - lastBitrateUpdate.msec()) / 1000000.0 >= 0.3) {
            lastBitrateUpdate = tm;
        }
        int bitrate = DBAPI->streamer_get_apx_bitrate();
        QString sbitrate;
        if (bitrate > 0) {
            sbitrate = QString("%1 kbps").arg(bitrate, 4);
        } else {
            sbitrate = "0 kbps";
        }
        QString spaused = DBAPI->get_output()->state() == OUTPUT_STATE_PAUSED ? QString(tr("Paused")) : QString(tr("Playing"));

        QString old_format(playingFormat);

        statusString = playingFormat.replace("%totaltime", totaltime_str);
        statusString = playingFormat.replace("%status", spaused);
        statusString = playingFormat.replace("%format", DBAPI->pl_find_meta(track, ":FILETYPE") ? DBAPI->pl_find_meta(track, ":FILETYPE") : "-");
        statusString = playingFormat.replace("%bitrate", sbitrate);
        statusString = playingFormat.replace("%samplerate", QString("%1Hz").arg(samplerate));
        statusString = playingFormat.replace("%bitspersample", QString("%1 bit").arg(bitspersample));
        statusString = playingFormat.replace("%channels", mode);
        statusString = playingFormat.replace("%curtime", QString("%1:%2").arg(minpos).arg(QString("%1").arg(secpos).rightJustified(2, '0')));
        statusString = playingFormat.replace("%duration", t);
        statusString = playingFormat.replace("%tracksnum", QString("%1").arg(DBAPI->pl_getcount(PL_MAIN)));

        playingFormat = old_format;
    }
    if (track) {
        DBAPI->pl_item_unref(track);
    }
    statusLabel.setText(statusString);
}
