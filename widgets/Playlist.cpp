#include "Playlist.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>

#include <QtGui.h>
#include <QtGuiSettings.h>

#include "MainWindow.h"

Playlist::Playlist(QWidget *parent, DBApi *Api) : QTreeView(parent), DBWidget(parent, Api), playlistModel(this, Api) {
    setAutoFillBackground(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setDragEnabled(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setIconSize(QSize(16, 16));
    setTextElideMode(Qt::ElideRight);
    setIndentation(0);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setItemsExpandable(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setWordWrap(false);
    setExpandsOnDoubleClick(false);
    setAcceptDrops(true);
    setModel(&playlistModel);

    header()->setStretchLastSection(false);

//###################################################
//     header()->setStretchLastSection(true);
//     header()->setSectionResizeMode(QHeaderView::Stretch);
//###################################################
    
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setSortIndicatorShown(false);
    
    header()->setDefaultSectionSize(80);
    header()->setMinimumSectionSize(10);
    
    createContextMenu();
    createHeaderContextMenu();
    createConnections();

    installEventFilter(this);

    playlistModel.setDefaultHeaders();
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    playlistModel.setPlaylist(plt);
    DBAPI->plt_unref(plt);
    //playlistModel.setColumns()
    QByteArray data = Api->confGetValue(parent->objectName(),"HeaderState",QByteArray()).toByteArray();
    qDebug() << _internalNameWidget << Qt::endl;
    if (data != QByteArray()) {
        header()->restoreState(data);
    }
    connect(api,SIGNAL(playlistChanged()),this,SLOT(onPlaylistChanged()));
}

QWidget * Playlist::constructor(QWidget *parent, DBApi *Api) {
    return new Playlist(parent, Api);
}

bool Playlist::eventFilter(QObject *target, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent *)event;
        if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
            emit enterRelease(currentIndex());
            return true;
        }
    }
    return QTreeView::eventFilter(target, event);
}

Playlist::~Playlist() {
    delete lockColumnsAction;
}

void Playlist::createConnections() {
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(enterRelease(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));

    connect(header(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(headerContextMenuRequested(QPoint)));
    connect(header(), SIGNAL(sectionResized(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionMoved(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionClicked(int)), SLOT(saveHeaderState()));
}

void Playlist::goToLastSelection() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int cursor = DBAPI->plt_get_cursor(plt, PL_MAIN);
    if (cursor < 0)
        restoreCursor();
    else
        setCurrentIndex(playlistModel.index(cursor, 0, QModelIndex()));
    DBAPI->plt_unref(plt);
}

void Playlist::restoreCursor() {
    int currentPlaylist = DBAPI->plt_get_curr_idx();
    int cursor = DBAPI->conf_get_int(QString("playlist.cursor.%1").arg(currentPlaylist).toUtf8().constData(), -1);
    setCurrentIndex(playlistModel.index(cursor, 0, QModelIndex()));
}

void Playlist::storeCursor() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int cursor = DBAPI->plt_get_cursor(plt, PL_MAIN);
    DBAPI->conf_set_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8().constData(), cursor);
    DBAPI->plt_unref(plt);
}

void Playlist::saveConfig() {
    SETTINGS->setValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderIsVisible,!header()->isHidden());
    SETTINGS->setValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderState, header()->saveState());
    SETTINGS->setValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderIsLocked, !header()->sectionsMovable() && header()->sectionResizeMode(1) == QHeaderView::Fixed);
    //playlistModel.saveConfig();
}

void Playlist::loadConfig() {
    bool isVisible = SETTINGS->getValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderIsVisible,true).toBool();
    bool isLocked = SETTINGS->getValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderIsLocked, true).toBool();
    headerState = SETTINGS->getValue(QtGuiSettings::PlayList, QtGuiSettings::HeaderState, QByteArray()).toByteArray();
    header()->setHidden(!isVisible);
    header()->restoreState(headerState);
    header()->setSectionsMovable(!isLocked);
    header()->setSectionResizeMode(isLocked ? QHeaderView::Fixed : QHeaderView::Interactive);
    lockColumnsAction->setChecked(isLocked);
}

void Playlist::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls() || event->mimeData()->hasFormat("playlist/track")
                                     || event->mimeData()->hasFormat("medialib/tracks")) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void Playlist::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        ddb_playlist_t *plt = DBAPI->plt_get_curr();
        int count = DBAPI->plt_get_item_count(plt, PL_MAIN);
        DBAPI->plt_unref(plt);
        int row = indexAt(event->pos()).row();
        int before = (row >= 0) ? row - 1 : count - 1;
        foreach (QUrl url, event->mimeData()->urls()) {
            playlistModel.insertByURLAtPosition(url, before);
            before++;
        }
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if (event->mimeData()->hasFormat("playlist/track")) {
        int row = indexAt(event->pos()).row();
        ddb_playlist_t *plt = DBAPI->plt_get_curr();
        int count = DBAPI->plt_get_item_count(plt, PL_MAIN);
        DBAPI->plt_unref(plt);
        row = (row >= 0) ? row : count;
        QByteArray encodedData = event->mimeData()->data("playlist/track");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        QHash<int,QString> newItems;
        while (!stream.atEnd()) {
            int row;
            QString text;
            stream >> row >> text;
            newItems[row] = text;
        }
        QList<int> rows = newItems.keys();
        std::sort(rows.begin(),rows.end());
        playlistModel.moveItems(rows, row);
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else if (event->mimeData()->hasFormat("medialib/tracks")) {
        QByteArray encodedData = event->mimeData()->data("medialib/tracks");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        playItemList a;
        stream >> a;
        //qDebug() <<"dropEven:" << a.list.at(0)->startsample << a.list.at(0)->endsample << a.list.at(0)->shufflerating << Qt::endl;
        qint64 i;
        ddb_playlist_t *plt = DBAPI->plt_get_curr();
        for (i = a.count-1; i >= 0; i--) {
            // TODO insert pos
            playlistModel.insertByPlayItemAtPosition(a.list.at(i),indexAt(event->pos()).row());
            //DBAPI->plt_insert_item(plt,nullptr,a.list.at(i));
        }
        DBAPI->plt_unref(plt);
        event->setDropAction(Qt::CopyAction);
        event->accept();
    } else {
        event->ignore();
    }
}

void Playlist::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if (selected == deselected)
        return;
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    DBAPI->plt_set_cursor(plt, PL_MAIN, selected.indexes().count() == 0 ? -1 : selected.indexes().last().row());
    DBAPI->plt_unref(plt);
    storeCursor();

    QTreeView::selectionChanged(selected, deselected);
}

void Playlist::trackDoubleClicked(QModelIndex index) {
    w->Api()->playTrackByIndex(index.row());
}

void Playlist::createContextMenu() {
    setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *delTrack = new QAction(tr("Remove Track(s) From Playlist"), this);
    delTrack->setShortcut(Qt::Key_Delete);
    connect(delTrack, SIGNAL(triggered()), this, SLOT(delSelectedTracks()));
    addAction(delTrack);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
}

void Playlist::createHeaderContextMenu() {
    lockColumnsAction = new QAction(tr("Lock columns"), &headerContextMenu);
    lockColumnsAction->setCheckable(true);
    lockColumnsAction->setChecked(header()->sectionsMovable() && header()->sectionResizeMode(0) == QHeaderView::Fixed);
    connect(lockColumnsAction, SIGNAL(toggled(bool)), this, SLOT(lockColumns(bool)));
    
    loadConfig();
    
    QMenu *columnsMenu = new QMenu(tr("Columns"), &headerContextMenu);
    foreach (PlaylistHeader_t *header, playlistModel.columns) {
        QAction *action = new QAction(header->title, &headerContextMenu);
        action->setCheckable(true);
        /*
        for (int i = 0; i < playlistModel.columns.count(); i++) {
            if (playlistModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == name ||
                (playlistModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == "" && name == tr("Status"))
            ) {
                action->setChecked(!isColumnHidden(i));
                break;
            }
            connect(action, SIGNAL(toggled(bool)), SLOT(setColumnHidden(bool)));
        }
        */
        columnsMenu->addAction(action);
    }
    headerContextMenu.addMenu(columnsMenu);
    headerContextMenu.addAction(lockColumnsAction);
}

void Playlist::showContextMenu(QPoint point) {
    if (indexAt(point).row() < 0)
        return;
    QMenu menu(this);
    menu.addActions(actions());
    menu.exec(mapToGlobal(point));
}

void Playlist::headerContextMenuRequested(QPoint pos) {
    headerContextMenu.move(mapToGlobal(pos));
    headerContextMenu.show();
}

void Playlist::lockColumns(bool locked) {
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);
    headerState = header()->saveState();
}

void Playlist::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    int index = DBAPI->plt_get_item_idx(plt, to, PL_MAIN);
    DBAPI->plt_unref(plt);
    setCurrentIndex(playlistModel.index(index, 0, QModelIndex()));

    playlistModel.index(index, 0, QModelIndex());
}

void Playlist::delSelectedTracks() {
    playlistModel.deleteTracks(selectionModel()->selectedRows());
}

void Playlist::clearPlayList() {
    playlistModel.clearPlayList();
}

void Playlist::insertByURLAtPosition(const QUrl& url, int position) {
    playlistModel.insertByURLAtPosition(url, position);
}

void Playlist::toggleHeaderHidden() {
    setHeaderHidden(!isHeaderHidden());
}

void Playlist::setColumnHidden(bool hidden) {
    if (QAction *action = qobject_cast<QAction *>(QObject::sender())) {
        for (int i = 0; i < header()->count(); i++) {
            if (playlistModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == action->text() ||
                (playlistModel.headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() == "" && action->text() == tr("Status"))
            ) {
                QTreeView::setColumnHidden(i, !hidden);
                headerState = header()->saveState();
                break;
            }
        }
    }
}

void Playlist::saveHeaderState() {
    qDebug() << _internalNameWidget << Qt::endl;
    headerState = header()->saveState();
    qDebug() << headerState << Qt::endl;

    api->confSetValue(_internalNameWidget, "HeaderState",headerState);
}

void Playlist::onPlaylistChanged() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    playlistModel.setPlaylist(plt);
}
