#include "QtGuiSettings.h"
#include <QtDebug>

QtGuiSettings *settings;

const QString QtGuiSettings::MainWindow = QString("MainWindow");
const QString QtGuiSettings::WindowSize = QString("WindowSize");
const QString QtGuiSettings::WindowPosition = QString("WindowPosition");
const QString QtGuiSettings::WindowState = QString("WindowState");
const QString QtGuiSettings::ToolbarsIsLocked = QString("ToolbarsIsLocked");
const QString QtGuiSettings::MainMenuIsHidden = QString("MainMenuIsHidden");
const QString QtGuiSettings::StatusbarIsHidden = QString("StatusbarIsHidden");
const QString QtGuiSettings::MinimizeOnClose = QString("MinimizeOnClose");
const QString QtGuiSettings::RefreshRate = QString("RefreshRate");
const QString QtGuiSettings::TitlebarPlaying = QString("TitlebarPlaying");
const QString QtGuiSettings::TitlebarStopped = QString("TitlebarStopped");
const QString QtGuiSettings::CoverartIsHidden = QString("CoverartIsHidden");
const QString QtGuiSettings::TrayIcon = QString("TrayIcon");
const QString QtGuiSettings::TrayIconIsHidden = QString("TrayIconIsHidden");
const QString QtGuiSettings::ShowTrayTips = QString("ShowTrayTips");
const QString QtGuiSettings::MessageFormat = QString("MessageFormat");
const QString QtGuiSettings::VolumeBar = QString("VolumeBar");
const QString QtGuiSettings::SeekSlider = QString("SeekSlider");
const QString QtGuiSettings::TabBarPosition = QString("TabBarPosition");
const QString QtGuiSettings::TabBarIsVisible = QString("TabBarIsVisible");
const QString QtGuiSettings::StatusBar = QString("StatusBar");
const QString QtGuiSettings::PlayingFormat = QString("PlayingFormat");
const QString QtGuiSettings::PausedFormat = QString("PausedFormat");
const QString QtGuiSettings::StoppedFormat = QString("StoppedFormat");
const QString QtGuiSettings::PlayList = QString("PlayList");
const QString QtGuiSettings::HeaderState = QString("HeaderState");
const QString QtGuiSettings::HeaderIsLocked = QString("HeaderIsLocked");
const QString QtGuiSettings::HeaderIsVisible = QString("HeaderIsVisible");


QtGuiSettings::QtGuiSettings(QObject *parent) : QSettings(parent) {
    // dummy
}

QVariant QtGuiSettings::getValue(const QString &group, const QString &key, const QVariant &defaultValue) {
    beginGroup(group);
    QVariant result = value(key, defaultValue);
    endGroup();
    return result;
}

void QtGuiSettings::setValue(const QString &group, const QString &key, const QVariant &value) {
    beginGroup(group);
    QSettings::setValue(key, value);
    endGroup();
}
