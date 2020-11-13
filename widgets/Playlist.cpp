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
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setSortIndicatorShown(false);
    
    // Context menu (TODO into core)
    setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *delTrack = new QAction(tr("Remove Track(s) From Playlist"), this);
    delTrack->setShortcut(Qt::Key_Delete);
    connect(delTrack, SIGNAL(triggered()), this, SLOT(delSelectedTracks()));
    addAction(delTrack);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    // Header lock
    bool locked = api->confGetValue(_internalNameWidget,"HeadersLocked", false).toBool();
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);

    // Header Menu
    headerActions.append(new QAction(_("Add column"),&headerContextMenu));
    headerActions.append(new QAction(_("Edit column"),&headerContextMenu));
    headerActions.append(new QAction(_("Remove column"),&headerContextMenu));
    connect(headerActions[0], SIGNAL(triggered(bool)), this, SLOT(headerDialogAdd(bool)));
    connect(headerActions[1], SIGNAL(triggered(bool)), this, SLOT(headerDialogEdit(bool)));
    connect(headerActions[2], SIGNAL(triggered(bool)), this, SLOT(headerDialogRemove(bool)));
    headerContextMenu.addActions(headerActions);
    headerContextMenu.addSeparator();
    headerActions.append(new QAction(_("Lock header bar"),&headerContextMenu));
    headerActions[3]->setCheckable(true);
    headerActions[3]->setChecked(locked);
    connect(headerActions[3], SIGNAL(triggered(bool)), this, SLOT(lockColumns(bool)));
    headerContextMenu.addAction(headerActions[3]);
    headerActions.append(new QAction(_("Lock playlist"),&headerContextMenu));
    headerActions[4]->setCheckable(true);
    headerActions[4]->setChecked(false);
    // TODO implement such feature
    headerActions[4]->setEnabled(false);
    headerContextMenu.addAction(headerActions[4]);

    headerGrouping = headerContextMenu.addMenu(_("Grouping"));
    headerGrouping->setEnabled(false);
    //TODO

    // Connections (kinda old)
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));
    connect(this, SIGNAL(enterRelease(QModelIndex)), SLOT(trackDoubleClicked(QModelIndex)));

    connect(header(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(headerContextMenuRequested(QPoint)));
    connect(header(), SIGNAL(sectionResized(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionMoved(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionClicked(int)), SLOT(saveHeaderState()));

    // this connection new :)
    connect(api,SIGNAL(playlistChanged()),this,SLOT(onPlaylistChanged()));

    // Event filter for enter
    installEventFilter(this);

    // Restore Header
    QByteArray ar = api->confGetValue(_internalNameWidget,"HeaderData",QVariant()).toByteArray();
    QDataStream ds(ar);
    while (!ds.atEnd()) {
        PlaylistHeader_t *ptr = new PlaylistHeader_t;
        headers.append(ptr);
        ds >> *ptr;
    }
    if (headers.length())
        playlistModel.setColumns(headers);
    else {
        headers = *playlistModel.setDefaultHeaders();
    }
    QByteArray data = Api->confGetValue(parent->objectName(),"HeaderState",QByteArray()).toByteArray();
    if (data != QByteArray()) {
        header()->restoreState(data);
    }

    // Set playlist
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    playlistModel.setPlaylist(plt);
    DBAPI->plt_unref(plt);
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
    // free header actions
    int i;
    for (i = 0; i < headerActions.length(); i++) {
        delete headerActions.at(i);
    }
    // maybe free headers?
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
    api->playTrackByIndex(index.row());
}

void Playlist::showContextMenu(QPoint point) {
    if (indexAt(point).row() < 0)
        return;
    QMenu menu(this);
    menu.addActions(actions());
    menu.exec(mapToGlobal(point));
}

void Playlist::headerContextMenuRequested(QPoint pos) {
    headerMenu_pos =  header()->logicalIndexAt(pos);
    headerContextMenu.move(mapToGlobal(pos));
    headerContextMenu.show();
}

void Playlist::lockColumns(bool locked) {
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);
    saveHeaderState();
}

void Playlist::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    // follow track option to add
    Q_UNUSED(from)
    Q_UNUSED(to)

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

void Playlist::saveHeaderState() {
    // HeaderState
    QByteArray headerState = header()->saveState();
    api->confSetValue(_internalNameWidget, "HeaderState",headerState);
    // HeaderData
    int i;
    QByteArray ar;
    QDataStream ds(&ar, QIODevice::WriteOnly);
    for (i = 0; i < headers.length(); i++) {
        ds << *headers.at(i);
    }
    api->confSetValue(_internalNameWidget,"HeaderData",ar);
}

void Playlist::onPlaylistChanged() {
    ddb_playlist_t *plt = DBAPI->plt_get_curr();
    playlistModel.setPlaylist(plt);
    DBAPI->plt_unref(plt);
}

void Playlist::headerDialogAdd(bool) {
    HeaderDialog a(this,headerMenu_pos,nullptr);
    connect(&a,SIGNAL(headerDialogEvent(int, PlaylistHeader_t *)),this,SLOT(headerAdd(int, PlaylistHeader_t *)));
    a.exec();
}

void Playlist::headerDialogEdit(bool) {
    HeaderDialog a(this,headerMenu_pos,headers.at(headerMenu_pos));
    connect(&a,SIGNAL(headerDialogEvent(int, PlaylistHeader_t *)),this,SLOT(headerEdit(int, PlaylistHeader_t *)));
    a.exec();
}
void Playlist::headerDialogRemove(bool) {
    headers.removeAt(headerMenu_pos);
    playlistModel.setColumns(headers);
}

void Playlist::headerAdd(int n,PlaylistHeader_t *ph) {
    headers.insert(n+1,ph);
    playlistModel.setColumns(headers);
    saveHeaderState();
}
void Playlist::headerEdit(int n,PlaylistHeader_t *ph) {
    PlaylistHeader_t *old_ph = headers.at(n);
    if (old_ph->format != ph->format) {
        // update format if changed
        char *tf_comp = ph->_format_compiled;
        old_ph->format = ph->format;
        ph->_format_compiled = nullptr;
        DBAPI->tf_free(tf_comp);
        ph->_format_compiled = nullptr;
    }
    headers.replace(n,ph);
    delete old_ph;
    playlistModel.setColumns(headers);
    saveHeaderState();
}

HeaderDialog::HeaderDialog(QWidget *parent, int headernum, PlaylistHeader_t *header) : QDialog(parent) {
    // headernum - column to insert (to be passed in signal)
    // header - pointer if modifying, otherwise null
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(405);

    if (header) {
        h = new PlaylistHeader_t;
        *h = *header;
        editting = 1;
        setWindowTitle(_("Edit column"));
    }
    else {
        h = new PlaylistHeader_t;
        // check if correct
        setWindowTitle(_("Add column"));
    }
    n = headernum;

    // layout
    this->setLayout(&layout);
    layout.addRow(_("Title:"), &title);
    layout.addRow(_("Type:"), &type);
    QStringList items = {_("Item Index"), _("Playing"), _("Album Art"), _("Artist - Album"),
                         _("Artist"), _("Album"), _("Title"), _("Year"), _("Duration"), _("Track Number"),
                         _("Band / Album Artist"), _("Codec"), _("Bitrate"), _("Custom")};
    type.addItems(items);
    format.setEnabled(false);
    format_parent.setLayout(&format_layout);
    format_layout.addWidget(&format);
    format_layout.addWidget(&format_help);
    format_help.setText(QString("<a href=\"http://example.com/\">%1</a>").arg(_("Help")));
    format_help.setTextFormat(Qt::RichText);
    format_help.setTextInteractionFlags(Qt::TextBrowserInteraction);
    format_help.setOpenExternalLinks(false);
    layout.addRow(_("Format:"), &format_parent);

    // Missing sort formatting, alignment, color

    // For edit
    if (editting) {
        title.setText(header->title);
        type.setCurrentIndex(header->type-1);
        if (type.currentIndex() + 1 == HT_custom) {
            format.setEnabled(true);
        }
        format.setText(header->format);
    }
    else {
        title.setText(items[0]);
        type.setCurrentIndex(0);
        h->title = items[0];
        h->type = HT_itemIndex;
    }
   //QDialogButtonBox *buttons = new QDialogButtonBox();
    buttons.addButton(QDialogButtonBox::Ok);
    buttons.addButton(QDialogButtonBox::Cancel);
    layout.addRow("",&buttons);

    connect (&title, SIGNAL(textEdited(const QString &)), this, SLOT(titleEdited(const QString &)));
    connect (&format, SIGNAL(textEdited(const QString &)), this, SLOT(formatEdited(const QString &)));
    connect (&type, SIGNAL(currentIndexChanged(int)), this, SLOT(typeChanged(int)));
    connect(&buttons, SIGNAL(accepted()), this, SLOT(accepted()));
    connect(&buttons, SIGNAL(rejected()), this, SLOT(rejected()));
}

void HeaderDialog::titleEdited(const QString &text) {
    h->title = text;
}
void HeaderDialog::typeChanged(int index) {
    if (index + 1 == HT_custom) {
        format.setEnabled(true);
        format.setText(format_saved);
    }
    else {
        format_saved = format.text();
        format.setEnabled(false);
        format.setText(PlaylistModel::formatFromHeaderType(static_cast<headerType>(index + 1)));
        if (type.itemText(h->type - 1) == title.text()) {
            title.setText(PlaylistModel::titleFromHeaderType(static_cast<headerType>(index + 1)));
        }
    }
    h->type = static_cast<headerType>(index + 1);
}
void HeaderDialog::formatEdited(const QString &text) {
    h->format = text;
}

void HeaderDialog::accepted() {
    emit headerDialogEvent(n,h);
    close();
}

void HeaderDialog::rejected() {
    delete h;
    close();
}
