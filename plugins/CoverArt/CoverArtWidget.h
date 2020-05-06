#ifndef COVERARTWIDGET_H
#define COVERARTWIDGET_H

#include <QDockWidget>
#include <QLabel>

#include "DBApi.h"
#include <QAction>

class CoverArtWidget : public QDockWidget {
    Q_OBJECT

public:
    CoverArtWidget(QWidget *parent = 0);
    ~CoverArtWidget();

    void updateCover(DB_playItem_t *track = NULL);

private:
    QLabel label;
    QAction updateCoverAction;

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
    
private Q_SLOTS:
    void trackChanged(DB_playItem_t *, DB_playItem_t *);
    void setCover(const QImage &);
    void reloadCover();

Q_SIGNALS:
    void onCloseEvent();
    
};

#endif // COVERARTWIDGET_H

