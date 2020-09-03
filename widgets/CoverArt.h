#ifndef COVERARTWIDGET_H
#define COVERARTWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include "DBApi.h"
#include <QAction>

class CoverArt : public QWidget, public DBWidget{
    Q_OBJECT

public:
    CoverArt(QWidget *parent = nullptr, DBApi *api = nullptr);
    ~CoverArt();

    QMargins *m;
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *Api = nullptr);

    QLabel *cover_display = nullptr;
    QImage *cover_image = nullptr;
    void updateCover(DB_playItem_t *track = NULL);

private:
    virtual void resizeEvent(QResizeEvent *event);
    QHBoxLayout *layout = nullptr;
    uint32_t handle_watch;
    
public slots:
    void onCurrCoverChanged(uint32_t);
    void onCoverLoaded(uint32_t, QImage *);

signals:
    void onCloseEvent();
    
};

#endif // COVERARTWIDGET_H

