#include "PlaylistBrowser.h"

#include <QStyle>
#include <QLayout>
#include <QtGui>

PlaylistBrowser::PlaylistBrowser(QWidget * parent, DBApi *Api) : QListView(parent), DBWidget(parent, Api) {
    // Fill list
    pbm = new PlaylistBrowserModel(nullptr, Api);
    setModel(pbm);

    // Connections
    connect (selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onCurrentChanged(QModelIndex,QModelIndex)));
    connect (api, SIGNAL(playlistChanged(int)), this, SLOT(selectPlaylist(int)));

    // Stylesheet
    this->setStyleSheet(QString("QListView::item {padding: 5px}"));

    // Drag'n'drop
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::InternalMove);
}

PlaylistBrowser::~PlaylistBrowser() {
    delete pbm;
}

void PlaylistBrowser::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QListView::mousePressEvent(event);
        return;
    }
    else if (event->button() == Qt::RightButton) {
        api->playlistContextMenu(this,event->pos(),indexAt(event->pos()).row());
        event->accept();
    }
}

QWidget *PlaylistBrowser::constructor(QWidget *parent, DBApi *Api) {
    return new PlaylistBrowser(parent, Api);
}

void PlaylistBrowser::selectPlaylist(int pl) {
    selectionModel()->select(model()->index(pl,0), QItemSelectionModel::ClearAndSelect);
}

void PlaylistBrowser::onCurrentChanged(const QModelIndex &to, const QModelIndex &from) {
    Q_UNUSED(from)
    api->changePlaylist(to.row());
}
