#include "PlaylistBrowser.h"

#include <QStyle>
#include <QtGui>

PlaylistBrowser::PlaylistBrowser(QWidget * parent, DBApi *Api) : QListWidget(parent), DBWidget(parent, Api) {
    // Fill list
    int count = api->getPlaylistCount();
    for (int i = 0; i < count; i++) {
        addItem(api->playlistNameByIdx(i));
    }
    setCurrentRow(api->deadbeef->plt_get_curr_idx());

    // Connections
    connect (this, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(onItemClicked(QListWidgetItem *)));
    connect (this, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(onItemClicked(QListWidgetItem *)));
    connect (this, SIGNAL(playlistSelected(int)), api, SLOT(changePlaylist(int)));
    connect (api, SIGNAL(playlistChanged(int)), this, SLOT(selectPlaylist(int)));
    connect (api, SIGNAL(playlistMoved(int, int)), this, SLOT(playlistOrderChanged(int, int)));
    //connect (api, SIGNAL(playlistRenamed(int)), this, SLOT(playlistRenamed(int)));

    // Stylesheet
    this->setStyleSheet(QString("QListView::item {padding: 5px}"));

    // Drag'n'drop
    this->setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
}

void PlaylistBrowser::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QListWidget::mousePressEvent(event);
    }
    else {
        event->accept();
        return;
    }
}

QWidget *PlaylistBrowser::constructor(QWidget *parent, DBApi *Api) {
    return new PlaylistBrowser(parent, Api);
}

QDockWidget *PlaylistBrowser::constructorDockWidget(QWidget *parent, DBApi *Api) {
    QWidget *pb = new PlaylistBrowser(parent, Api);
    QDockWidget *dw = new QDockWidget(QString("Playlist Browser"));

    dw->setWidget(pb);
    return dw;
}

void PlaylistBrowser::selectPlaylist(int pl) {
    setCurrentItem(item(pl));
}

void PlaylistBrowser::onItemClicked(QListWidgetItem *item) {
    emit playlistSelected(row(item));
}

void PlaylistBrowser::dropEvent(QDropEvent *event) {
    QListWidgetItem *it = currentItem();
    int pos_before = row(it);
    QListWidget::dropEvent(event);
    int pos_after = row(it);

    if (pos_before != pos_after) {
        // move happened
        our_pl = pos_before;
        our_before = pos_after;
        api->movePlaylist(pos_before, pos_after);
    }
}

void PlaylistBrowser::playlistOrderChanged(int pl, int before) {
    if (pl == our_pl && before == our_before) {
        // Ignore order change done by this widget
        our_pl = -1;
        our_before = -1;
        return;
    }
    QListWidgetItem *currentItem = takeItem(pl);
    insertItem(before, currentItem);
}

void PlaylistBrowser::playlistRenamed(int plt) {
    this->item(plt)->setText(api->playlistNameByIdx(plt));
}
