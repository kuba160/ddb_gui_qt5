#ifndef QTGUISETTINGS_H
#define QTGUISETTINGS_H

#define SETTINGS settings

#include <QVariant>
#include <QString>
#include <QSettings>

#define QSSETCONF(X,Y) autoSetValue(this, X, Y);
#define QSGETCONF(X,Y) autoGetValue(this, X, Y);


class QtGuiSettings : public QSettings {
    Q_OBJECT
public:
    QtGuiSettings(QObject *parent);
    //QtGuiSettings(const QString &fileName, QSettings::Format format, QObject *parent = nullptr);
    //~QtGuiSettings();

    QVariant getValue(const QString &group, const QString &key, const QVariant &defaultValue);
    void setValue(const QString &group, const QString &key, const QVariant &value);

    // use shorter version QSSETCONF(key, value) and QSGETCONF(key, defvalue);
    void autoSetValue(void *, const QString &key, const QVariant &value);
    QVariant autoGetValue(void *, const QString &key, const QVariant &value);

    void removeValue(const QString &group, const QString &key);

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
    static const QString CoverartIsHidden;

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
    //static QtGuiSettings *instance;

    //QSettings settings;
};

extern QtGuiSettings *settings;

#endif // QTGUISETTINGS_H
