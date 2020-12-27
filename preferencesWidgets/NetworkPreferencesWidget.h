#ifndef NETWORKPREFERENCESWIDGET_H
#define NETWORKPREFERENCESWIDGET_H

#include <QWidget>

namespace Ui {
    class NetworkPreferencesWidget;
}

class NetworkPreferencesWidget : public QWidget {
    Q_OBJECT
public:
    NetworkPreferencesWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Window);
private:
    Ui::NetworkPreferencesWidget *ui;

    void loadSettings();
    void createConnections();
    
protected:
    void changeEvent(QEvent *e);
    
private Q_SLOTS:
    void enableProxy(bool);
    void saveProxyAddress();
    void saveProxyPassword();
    void saveProxyPort();
    void saveProxyType(int);
    void saveProxyUsername();
};

#endif // NETWORKPREFERENCESWIDGET_H
