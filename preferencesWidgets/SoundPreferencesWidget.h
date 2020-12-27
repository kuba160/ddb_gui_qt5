#ifndef SOUNDPREFERENCESWIDGET_H
#define SOUNDPREFERENCESWIDGET_H

#include <QWidget>
#include <QHash>

namespace Ui {
    class SoundPreferencesWidget;
}

class SoundPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    SoundPreferencesWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::Window);
    
    void addDevice(const char *name, const char *desc);
    
private:
    Ui::SoundPreferencesWidget *ui;
    
    QHash<QString, QString> alsaDevices;
    
    void loadSettings();
    void createConnections();
    
protected:
    void changeEvent(QEvent *e);

private Q_SLOTS:
    void changeOutputDevice(int);
    void changeOutputPlugin(int);
    void changeReplaygainMode(int);
    void saveReplaygainScale(bool);
    void saveReplaygainPreamp();
    void saveAddToDefaultPlaylist(bool);
    void saveDefaultPlaylistName();
    void saveDontAddArchives(bool);
    void saveResumeOnStartup(bool);
};

#endif // SOUNDPREFERENCESWIDGET_H
