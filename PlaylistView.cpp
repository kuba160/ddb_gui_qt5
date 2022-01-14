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


AutoToolTipDelegate::AutoToolTipDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

bool AutoToolTipDelegate::helpEvent(QHelpEvent* e, QAbstractItemView* view, const QStyleOptionViewItem& option,
                                    const QModelIndex& index) {
    if (!e || !view) {
        return false;
    }
    if (e->type() == QEvent::ToolTip) {
        QRect rect = view->visualRect(index);
#if QT_VERSION > QT_VERSION_CHECK(5,11,0)
        int px = QFontMetrics(view->font()).horizontalAdvance(index.data(Qt::DisplayRole).toString().append(".."));
#else
        int px = QFontMetrics(view->font()).width(index.data(Qt::DisplayRole).toString().append(".."));
#endif
        // calculate identation
        QModelIndex ident_scan = index;
        int par = 0;
        while (ident_scan.parent().isValid()) {
            ident_scan = ident_scan.parent();
            // todo more precise calculation
            par++;
        }
        px += par >= 2 ? 55 : par == 1 ? 62 : 0;
        if (rect.width() < px) {
            QVariant tooltip = index.data(Qt::DisplayRole);
            if (tooltip.canConvert<QString>()) {
                QToolTip::showText(e->globalPos(),tooltip.toString(),view);
                return true;
            }
        }
        if (!QStyledItemDelegate::helpEvent(e, view, option, index)) {
            QToolTip::hideText();
        }
        return true;
    }
    return QStyledItemDelegate::helpEvent(e, view, option, index);
}

PlaylistView::PlaylistView(QWidget *parent, DBApi *Api, PlayItemTableModel *ptm) : QTreeView(parent),
                                                                              DBWidget(parent, Api) {
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
    setStyle(new myViewStyle);
    setItemDelegate(new AutoToolTipDelegate(this));

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
    //connect(delTrack, SIGNAL(triggered()), this, SLOT(delSelectedTracks()));
    addAction(delTrack);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    // Header lock
    bool locked = api->confGetValue(_internalNameWidget,"HeadersLocked", false).toBool();
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);

    // Set Model
    setModel(ptm);
    pi_model = ptm;

    // Restore Header
    QVariant var = api->confGetValue(_internalNameWidget,"HeaderData",QVariant());
    if (var.isValid() && var.canConvert<QByteArray>()) {
        QByteArray ar = var.toByteArray();
        ptm->setHeaderSettings(ar);
    }
    else {
        ptm->setDefaultHeaderSettings();
    }

    QByteArray data = Api->confGetValue(_internalNameWidget,"HeaderState",QByteArray()).toByteArray();
    if (data != QByteArray()) {
        header()->restoreState(data);
    }

    // Header Menu
    headerActions.append(new QAction(QIcon::fromTheme("list-add"),tr("Add column"),&headerContextMenu));
    headerActions.append(new QAction(QIcon::fromTheme("edit-entry"),tr("Edit column"),&headerContextMenu));
    headerActions.append(new QAction(QIcon::fromTheme("list-remove"),tr("Remove column"),&headerContextMenu));
    connect(headerActions[0], SIGNAL(triggered(bool)), this, SLOT(onHeaderDialogAdd()));
    connect(headerActions[1], SIGNAL(triggered(bool)), this, SLOT(onHeaderDialogEdit()));
    connect(headerActions[2], SIGNAL(triggered(bool)), this, SLOT(onHeaderDialogRemove()));
    headerContextMenu.addActions(headerActions);
    headerContextMenu.addSeparator();
    headerActions.append(new QAction(tr("Lock header bar"),&headerContextMenu));
    headerActions[3]->setCheckable(true);
    headerActions[3]->setChecked(locked);
    connect(headerActions[3], SIGNAL(triggered(bool)), this, SLOT(lockColumns(bool)));
    headerContextMenu.addAction(headerActions[3]);
    headerActions.append(new QAction(tr("Lock playlist"),&headerContextMenu));
    connect(headerActions[4], SIGNAL(triggered(bool)), this, SLOT(lockPlaylist(bool)));
    headerActions[4]->setCheckable(true);
    headerActions[4]->setChecked(false);
    // save lock playlist
    headerContextMenu.addAction(headerActions[4]);

    headerGrouping = headerContextMenu.addMenu(tr("Group By ..."));
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

    // Actions provider
    QActionGroup *ag = new QActionGroup(this);
    ag->setExclusive(false);
    add_to_playback_queue = ag->addAction(tr("Add To Playback Queue"));
    add_to_playback_queue->setObjectName("add_to_playback_queue");
    add_to_playback_queue->setIcon(QIcon::fromTheme("list-add"));
    connect(add_to_playback_queue, SIGNAL(triggered()), this, SLOT(onAddToPlaybackQueue()));
    remove_from_playback_queue = ag->addAction(tr("Remove From Playback Queue"));
    remove_from_playback_queue->setObjectName("remove_from_playback_queue");
    connect(remove_from_playback_queue, SIGNAL(triggered()), this, SLOT(onRemoveFromPlaybackQueue()));
    cut = ag->addAction(tr("Cut"));
    cut->setObjectName("cut");
    connect(cut,SIGNAL(triggered()),this,SLOT(onCut()));
    copy = ag->addAction(tr("Copy"));
    copy->setObjectName("copy");
    connect(copy,SIGNAL(triggered()),this,SLOT(onCopy()));
    paste = ag->addAction(tr("Paste"));
    paste->setObjectName("paste");
    connect(paste,SIGNAL(triggered()),this,SLOT(onPaste()));
    delete_action = ag->addAction(tr("Delete"));
    delete_action->setObjectName("delete");
    connect(delete_action, SIGNAL(triggered()), this, SLOT(onDelete()));
    setProperty("Actions",((quintptr) ag));
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

void PlaylistView::restoreCursor() {
    // TODO
    //int currentPlaylist = DBAPI->plt_get_curr_idx();
    //int cursor = DBAPI->conf_get_int(QString("playlist.cursor.%1").arg(currentPlaylist).toUtf8().constData(), -1);
    //setCurrentIndex()
    //setCurrentIndex(playlistModel.index(cursor, 0, QModelIndex()));
}

void PlaylistView::storeCursor() {
    // TODO
    //ddb_playlist_t *plt = playlistModel.getPlaylist();
    //int cursor = DBAPI->plt_get_cursor(plt, PL_MAIN);
    //DBAPI->conf_set_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8().constData(), cursor);
    //DBAPI->plt_unref(plt);
}

void PlaylistView::onCut() {
    //
    onCopy();
    QModelIndexList qmil = selectionModel()->selectedRows();
    if (qmil.length()) {
        api->clearClipboard();
        playItemList l = pi_model->tracks(qmil);
        api->clipboard->setMimeData(api->mime_playItemsCopy(l));
        // todo call model
        api->removeTracks(l);
    }
}


void PlaylistView::onCopy() {
    QModelIndexList qmil = selectionModel()->selectedRows();
    if (qmil.length() == 0) {
        return;
    }
    api->clearClipboard();
    playItemList l = pi_model->tracks(qmil);
    api->clipboard->setMimeData(api->mime_playItems(l));
}



void PlaylistView::onPaste() {
    if (api->clipboard->mimeData()->hasFormat("deadbeef/playitems")) {
        QDropEvent *event = new QDropEvent(mapFromGlobal(menu_pos),Qt::CopyAction,api->clipboard->mimeData(),Qt::NoButton,Qt::NoModifier);
        dropEvent(event);
        api->clipboard->clear();
        delete event;
    }
}

void PlaylistView::onDelete() {
    // TODO confirmation?
    QList<int> l;
    foreach(QModelIndex i, selectedIndexes()) {
        if (!l.contains(i.row())) {
            l.append(i.row());
        }
    }
    pi_model->removeIndexes(l);
}

void PlaylistView::startDrag(Qt::DropActions supportedActions) {
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = model()->mimeData(selectedIndexes());
    drag->setMimeData(mimeData);
    drag->exec(supportedActions);
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
    if (event->mimeData()->hasFormat("deadbeef/playitems")) {
        int row = indexAt(event->pos()).row();
        // Adjust position based on indicator
        if (row == -1) {
            row = -2;
        }
        if(row != -1 && dropIndicatorPosition() == QAbstractItemView::AboveItem) {
            row--;
        }
        if (event->source() == this) {
            // Move items inside
            QList<DB_playItem_t *>list = api->mime_playItems(event->mimeData());
            QList<int> rows;
            for (int i = 0; i < list.length(); i++) {
                rows.append(DBAPI->pl_get_idx_of(list[i]));
            }
            pi_model->moveIndexes(rows,row);
        }
        else {
            // Insert foreign items
            QList<DB_playItem_t *>list;
            if (_internalNameWidget == "queuemanager") {
                // hack
                list = api->mime_playItems(event->mimeData());
            }
            else {
                list = api->mime_playItemsCopy(event->mimeData());
            }
            pi_model->insertTracks(&list,row);
            foreach(DB_playItem_t *it,list) {
                DBAPI->pl_item_unref(it);
            }
        }
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void PlaylistView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    if (selected == deselected)
        return;
    //TODO
    //ddb_playlist_t *plt = playlistModel.getPlaylist();
    //DBAPI->plt_set_cursor(plt, PL_MAIN, selected.indexes().count() == 0 ? -1 : selected.indexes().last().row());
    //DBAPI->plt_unref(plt);
    storeCursor();

    QTreeView::selectionChanged(selected, deselected);
}



void PlaylistView::showContextMenu(QPoint point) {
    QModelIndexList selection = selectedIndexes();
    playItemList l = pi_model->tracks(selection);
    setProperty("playItemsSelected", l.length());

    // update actions
    bool hasTracks = l.length();

    add_to_playback_queue->setEnabled(hasTracks);
    remove_from_playback_queue->setEnabled(false);
    cut->setEnabled(hasTracks);
    copy->setEnabled(hasTracks);
    paste->setEnabled(api->clipboard->mimeData()->hasFormat("deadbeef/playitems"));
    delete_action->setEnabled(hasTracks);

    // remove from playback check
    if (!this->property("queue_remove_always_enabled").toBool()) {
        foreach (DB_playItem_t *it, l) {
            if(DBAPI->playqueue_test(it) != -1) {
                remove_from_playback_queue->setEnabled(true);
                break;
            }
        }
    }

    foreach(DB_playItem_t *it, l) {
        DBAPI->pl_item_unref(it);
    }
    menu_pos = point;
    api->playItemContextMenu(this, viewport()->mapToGlobal(point));
}

void PlaylistView::headerContextMenuRequested(QPoint pos) {
    headerMenu_pos =  header()->logicalIndexAt(pos);

    headerActions[1]->setEnabled(headerMenu_pos == -1 ? false : true);
    headerActions[2]->setEnabled(headerMenu_pos == -1 ? false : true);
    headerContextMenu.move(mapToGlobal(pos));
    headerContextMenu.show();
}

void PlaylistView::lockColumns(bool locked) {
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);
    saveHeaderState();
}

void PlaylistView::lockPlaylist(bool locked) {
    pi_model->setPlaylistLock(locked);
}

void PlaylistView::onTrackChanged(DB_playItem_t *from, DB_playItem_t *to) {
    // follow track option to add
    Q_UNUSED(from)
    Q_UNUSED(to)

}

void PlaylistView::saveHeaderState() {
    // HeaderState
    QByteArray headerState = header()->saveState();
    api->confSetValue(_internalNameWidget, "HeaderState",headerState);
    // HeaderData
    QByteArray ar = pi_model->getHeaderSettings();
    api->confSetValue(_internalNameWidget,"HeaderData",ar);
}

void PlaylistView::onAddToPlaybackQueue() {
    playItemList l = pi_model->tracks(selectedIndexes());
    foreach(DB_playItem_t *it, l) {
        DBAPI->playqueue_push(it);
        DBAPI->pl_item_unref(it);

    }
}
void PlaylistView::onRemoveFromPlaybackQueue() {
    playItemList l = pi_model->tracks(selectedIndexes());
    foreach(DB_playItem_t *it, l) {
        DBAPI->playqueue_remove(it);
        DBAPI->pl_item_unref(it);
    }
}

void PlaylistView::onHeaderDialogAdd() {
    HeaderDialog *dialog = new HeaderDialog(pi_model, this,-1);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(saveHeaderState()));
    dialog->open();
}

void PlaylistView::onHeaderDialogEdit() {
    HeaderDialog *dialog = new HeaderDialog(pi_model, this,headerMenu_pos);
    connect(dialog, SIGNAL(finished(int)), this, SLOT(saveHeaderState()));
    dialog->open();
}

void PlaylistView::onHeaderDialogRemove() {
    pi_model->removeHeader(headerMenu_pos);
    saveHeaderState();
}

HeaderDialog::HeaderDialog(PlayItemTableModel *pi_model, QWidget *parent, int headernum) : QDialog(parent) {
    // headernum - column to insert (to be passed in signal)
    // header - pointer if modifying, otherwise null
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(405);
    setAttribute(Qt::WA_DeleteOnClose);

    pt = pi_model;

    if (headernum  != -1) {
        setWindowTitle(tr("Edit column"));
    }
    else {
        setWindowTitle(tr("Add column"));
    }
    n = headernum;

    // layout
    this->setLayout(&layout);
    layout.addRow(tr("Title:"), &title);
    layout.addRow(tr("Type:"), &type);
    QStringList items;
    for (int i = PlayItemModel::ItemIndex; i <= PlayItemModel::LastRoleUnused; i++) {
        if (i == PlayItemModel::ItemPlaying) {
            items.append(tr("Playing"));
        }
        else {
            items.append(pt->defaultTitle(i));
        }
    }
    type.addItems(items);
    //format.setEnabled(false);
    format_parent.setLayout(&format_layout);
    format_layout.addWidget(&format);
    format_layout.addWidget(&format_help);
    format_help.setText(QString("<a href=\"https://github.com/DeaDBeeF-Player/deadbeef/wiki/Title-formatting-2.0\">%1</a>").arg(tr("Help")));
    format_help.setTextFormat(Qt::RichText);
    format_help.setTextInteractionFlags(Qt::TextBrowserInteraction);
    format_help.setOpenExternalLinks(true);
    layout.addRow(tr("Format:"), &format_parent);

    // Missing sort formatting, alignment, color

    if (n == -1) {
        // Add
        type.setCurrentIndex(0);
        title.setPlaceholderText(PlayItemModel::defaultTitle(PlayItemModel::ItemIndex));
    }
    else {
        // Edit

        // Trim for removing custom roles
        int role = qMin(pt->getHeaderRole(n),(int) PlayItemModel::LastRoleUnused);
        type.setCurrentIndex(role - PlayItemModel::ItemIndex);
        title.setText(pt->getHeaderTitle(n));
        title.setPlaceholderText(pt->defaultTitle(role));
        format.setText(pt->getHeaderFormat(n));

        if (role == PlayItemModel::LastRoleUnused) {
            format.setEnabled(true);
        }
        else {
            format.setEnabled(false);
        }
    }

    buttons.addButton(QDialogButtonBox::Ok);
    buttons.addButton(QDialogButtonBox::Cancel);
    layout.addRow("",&buttons);

    connect (&type, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
    connect(&buttons, SIGNAL(accepted()), this, SLOT(onAccepted()));
    connect(&buttons, SIGNAL(rejected()), this, SLOT(onRejected()));
}

void HeaderDialog::onTypeChanged(int idx_new) {
    if (idx_new + PlayItemModel::ItemIndex == PlayItemModel:: LastRoleUnused) {
        format.setEnabled(true);
    }
    else {
        format.setEnabled(false);
        format.setText(pt->defaultFormat(idx_new + PlayItemModel::ItemIndex));
    }
    title.setPlaceholderText(pt->defaultTitle(idx_new + PlayItemModel::ItemIndex));
}

void HeaderDialog::onAccepted() {
    if (n == -1) {
        // Add
        if(type.currentIndex() + PlayItemModel::ItemIndex == PlayItemModel::LastRoleUnused) {
            pt->addHeader(title.text(), format.text());
        }
        else {
            pt->addHeader(type.currentIndex() + PlayItemModel::ItemIndex, title.text());
        }
    }
    else {
        // Edit
        if(type.currentIndex() + PlayItemModel::ItemIndex == PlayItemModel::LastRoleUnused) {
            pt->modifyHeader(n, PlayItemModel::LastRoleUnused, title.text(), format.text());
        }
        else {
            pt->modifyHeader(n, type.currentIndex() + PlayItemModel::ItemIndex, title.text());
        }
    }
    close();
}

void HeaderDialog::onRejected() {
    close();
}
