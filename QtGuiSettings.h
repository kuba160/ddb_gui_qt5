#ifndef QTGUISETTINGS_H
#define QTGUISETTINGS_H

#define SETTINGS QtGuiSettings::Instance()

#include <QVariant>
#include <QString>
#include <QSettings>

class QtGuiSettings : public QObject {
    Q_OBJECT
public:
    static QtGuiSettings *Instance();
    static void Destroy();

    QVariant getValue(const QString &group, const QString &key, const QVariant &defaultValue);
    void setValue(const QString &group, const QString &key, const QVariant &value);

//     MainWindow Group
    static const QString MainWindow;
    
    static const QString WindowSize;
    static const QString WindowPosition;
    static const QString WindowState;
    static const QString ToolbarsIsLocked;
    static const QString MainMenuIsHidden;
    static const QString StatusbarIsHidden;
    static const QString MinimizeOnClose;
    static const QString RefreshRate;
    static const QString TitlebarPlaying;
    static const QString TitlebarStopped;
    static const QString TabBarPosition;
    static const QString TabBarIsVisible;
#ifdef ARTWORK_ENABLED
    static const QString CoverartIsHidden;
#endif

    //TrayIcon Group
    static const QString TrayIcon;

    static const QString TrayIconIsHidden;
    static const QString ShowTrayTips;
    static const QString MessageFormat;

    //VolumeBar & SeekSlider Groups
    static const QString VolumeBar;
    static const QString SeekSlider;

    //StatusBar Group
    static const QString StatusBar;
    
    static const QString PlayingFormat;
    static const QString PausedFormat;
    static const QString StoppedFormat;

    //PlayList Group
    static const QString PlayList;
    
    static const QString HeaderState;
    static const QString HeaderIsLocked;
    static const QString HeaderIsVisible;

private:
    QtGuiSettings();
    static QtGuiSettings *instance;

    QSettings settings;
};

#endif // QTGUISETTINGS_H
