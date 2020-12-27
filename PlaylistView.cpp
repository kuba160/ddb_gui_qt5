#include "PlaylistView.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>

#include <QtGui.h>
#include <QtGuiSettings.h>

#include "MainWindow.h"
#undef _
#include "DeadbeefTranslator.h"

class myViewStyle: public QProxyStyle{
public:
    myViewStyle(QStyle* style = 0);

    void drawPrimitive ( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const;
};

myViewStyle::myViewStyle(QStyle* style)
     :QProxyStyle(style)
{}

void myViewStyle::drawPrimitive ( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const{
    if (element == QStyle::PE_IndicatorItemViewItemDrop){
        QStyleOption opt(*option);
        opt.rect.setLeft(0);
        if (widget) {
            opt.rect.setRight(widget->width());
        }
        // TODO fix drawing when pos = -1
        QProxyStyle::drawPrimitive(element, &opt, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

PlaylistView::PlaylistView(QWidget *parent, DBApi *Api) : QTreeView(parent), DBWidget(parent, Api), playlistModel(this, Api) {
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
    viewport()->setAcceptDrops(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setModel(&playlistModel);
    setStyle(new myViewStyle);

    header()->setStretchLastSection(false);
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setSortIndicatorShown(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    header()->setFirstSectionMovable(true);
#endif

    
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
    connect(headerActions[4], SIGNAL(triggered(bool)), this, SLOT(lockPlaylist(bool)));
    headerActions[4]->setCheckable(true);
    headerActions[4]->setChecked(false);
    // save lock playlist
    headerContextMenu.addAction(headerActions[4]);

    headerGrouping = headerContextMenu.addMenu(_("Grouping"));
    headerGrouping->setEnabled(false);
    //TODO

    connect(header(), SIGNAL(customContextMenuRequested(QPoint)), SLOT(headerContextMenuRequested(QPoint)));
    connect(header(), SIGNAL(sectionResized(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionMoved(int,int,int)), SLOT(saveHeaderState()));
    connect(header(), SIGNAL(sectionClicked(int)), SLOT(saveHeaderState()));

    // this connection new :)
    //connect(api,SIGNAL(playlistChanged()),this,SLOT(onPlaylistChanged()));

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
}

bool PlaylistView::eventFilter(QObject *target, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = (QKeyEvent *)event;
        if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return)) {
            emit enterRelease(currentIndex());
            return true;
        }
    }
    return QTreeView::eventFilter(target, event);
}

PlaylistView::~PlaylistView() {
    // free header actions
    int i;
    for (i = 0; i < headerActions.length(); i++) {
        delete headerActions.at(i);
    }
    // maybe free headers?
}

void PlaylistView::goToLastSelection() {
    ddb_playlist_t *plt = playlistModel.getPlaylist();
    int cursor = DBAPI->plt_get_cursor(plt, PL_MAIN);
    if (cursor < 0)
        restoreCursor();
    else
        setCurrentIndex(playlistModel.index(cursor, 0, QModelIndex()));
    DBAPI->plt_unref(plt);
}

void PlaylistView::restoreCursor() {
    // TODO
    //int currentPlaylist = DBAPI->plt_get_curr_idx();
    //int cursor = DBAPI->conf_get_int(QString("playlist.cursor.%1").arg(currentPlaylist).toUtf8().constData(), -1);
    //setCurrentIndex(playlistModel.index(cursor, 0, QModelIndex()));
}

void PlaylistView::storeCursor() {
    // TODO
    //ddb_playlist_t *plt = playlistModel.getPlaylist();
    //int cursor = DBAPI->plt_get_cursor(plt, PL_MAIN);
    //DBAPI->conf_set_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8().constData(), cursor);
    //DBAPI->plt_unref(plt);
}

QMimeData *PlaylistView::copy() {
    //
    QList<DB_playItem_t *> list;
    QModelIndexList qmil = selectionModel()->selectedRows();
    int i;
    for (i = 0; i < qmil.length(); i++) {
        DB_playItem_t *it_new = DBAPI->pl_item_alloc();
        DBAPI->pl_item_copy(it_new, DBAPI->plt_get_item_for_idx(playlistModel.getPlaylist(), qmil[i].row(), PL_MAIN));
        list.append(it_new);
    }
    return api->mime_playItems(list);
}

bool PlaylistView::canCopy() {
    return true;
}

bool PlaylistView::canPaste(const QMimeData *mime) {
    if (mime->hasFormat("deadbeef/playitems")) {
        return true;
    }
    return false;
}

void PlaylistView::paste(const QMimeData *mime, QPoint p) {
    //
    QList<DB_playItem_t *> list = api->mime_playItems(mime);
    QDropEvent *event = new QDropEvent(mapFromGlobal(p),Qt::CopyAction,mime,Qt::NoButton,Qt::NoModifier);
    dropEvent(event);
}

void PlaylistView::startDrag(Qt::DropActions supportedActions) {
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = model()->mimeData(selectedIndexes());
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
    return;
}

void PlaylistView::dragMoveEvent(QDragMoveEvent* event) {

    QAbstractItemView::dragMoveEvent(event);
    QAbstractItemView::setDropIndicatorShown(true);
}

void PlaylistView::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls() || event->mimeData()->hasFormat("playlist/track")
                                     || event->mimeData()->hasFormat("deadbeef/playitems")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void PlaylistView::dropEvent(QDropEvent *event) {
    if (event->mimeData()->hasUrls()) {
        // TODO Deprecate this
        ddb_playlist_t *plt = playlistModel.getPlaylist();
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
        // TODO Deprecate this
        int row = indexAt(event->pos()).row();
        ddb_playlist_t *plt = playlistModel.getPlaylist();
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
    } else if (event->mimeData()->hasFormat("deadbeef/playitems")) {
        QList<DB_playItem_t *>list = api->mime_playItems(event->mimeData());
        int row = indexAt(event->pos()).row();
        // Adjust position based on indicator
        if(row != -1 && dropIndicatorPosition() == QAbstractItemView::BelowItem) {
            row++;
        }
        if (event->source() == this) {
            // Move items inside
            QList<int> rows;
            int i;
            for (i = 0; i < list.length(); i++) {
                rows.append(DBAPI->pl_get_idx_of(list[i]));
            }
            playlistModel.moveItems(rows,row);

        }
        else {
            // Insert foreign items
            if (row == -1) {
                row = model()->rowCount() - 1;
            }
            int i;
            for (i = 0; i < list.length(); i++) {
                playlistModel.insertByPlayItemAtPosition(list.at(i),row++);
            }
            event->setDropAction(Qt::CopyAction);
            event->accept();
        }
    } else {
        event->ignore();
    }
}

void PlaylistView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if (selected == deselected)
        return;
    ddb_playlist_t *plt = playlistModel.getPlaylist();
    DBAPI->plt_set_cursor(plt, PL_MAIN, selected.indexes().count() == 0 ? -1 : selected.indexes().last().row());
    DBAPI->plt_unref(plt);
    storeCursor();

    QTreeView::selectionChanged(selected, deselected);
}

void PlaylistView::trackDoubleClicked(QModelIndex index) {
    api->playTrackByIndex(index.row());
}

void PlaylistView::showContextMenu(QPoint point) {
    if (indexAt(point).row() < 0)
        return;
    api->playItemContextMenu(this, viewport()->mapTo(this,point));
}

void PlaylistView::headerContextMenuRequested(QPoint pos) {
    headerMenu_pos =  header()->logicalIndexAt(pos);
    headerContextMenu.move(mapToGlobal(pos));
    headerContextMenu.show();
}

void PlaylistView::lockColumns(bool locked) {
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);
    saveHeaderState();
}

void PlaylistView::lockPlaylist(bool locked) {
    playlistModel.setPlaylistLock(locked);
}

void PlaylistView::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    // follow track option to add
    Q_UNUSED(from)
    Q_UNUSED(to)

}

void PlaylistView::delSelectedTracks() {
    playlistModel.deleteTracks(selectionModel()->selectedRows());
}

void PlaylistView::clearPlayList() {
    playlistModel.clearPlayList();
}

void PlaylistView::insertByURLAtPosition(const QUrl& url, int position) {
    playlistModel.insertByURLAtPosition(url, position);
}

void PlaylistView::saveHeaderState() {
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

void PlaylistView::headerDialogAdd(bool) {
    HeaderDialog a(this,headerMenu_pos,nullptr);
    connect(&a,SIGNAL(headerDialogEvent(int, PlaylistHeader_t *)),this,SLOT(headerAdd(int, PlaylistHeader_t *)));
    a.exec();
}

void PlaylistView::headerDialogEdit(bool) {
    HeaderDialog a(this,headerMenu_pos,headers.at(headerMenu_pos));
    connect(&a,SIGNAL(headerDialogEvent(int, PlaylistHeader_t *)),this,SLOT(headerEdit(int, PlaylistHeader_t *)));
    a.exec();
}
void PlaylistView::headerDialogRemove(bool) {
    headers.removeAt(headerMenu_pos);
    playlistModel.setColumns(headers);
}

void PlaylistView::headerAdd(int n,PlaylistHeader_t *ph) {
    headers.insert(n+1,ph);
    playlistModel.setColumns(headers);
    saveHeaderState();
}
void PlaylistView::headerEdit(int n,PlaylistHeader_t *ph) {
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
    //format.setEnabled(false);
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
        if (type.currentIndex() + 1 != HT_custom) {
            format.setReadOnly(true);
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
        format.setReadOnly(false);
        format.setText(format_saved);
    }
    else {
        format_saved = format.text();
        format.setReadOnly(true);
        format.setText(PlaylistModel::formatFromHeaderType(static_cast<headerType>(index + 1)));
        if (type.itemText(h->type - 1) == title.text()) {
            title.setText(PlaylistModel::titleFromHeaderType(static_cast<headerType>(index + 1)));
            h->title = title.text();
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
