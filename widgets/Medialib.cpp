#include <QTreeWidget>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
// Dialog
#include <QDialog>
#include <QListView>
#include <QPushButton>
#include <QFileDialog>

#include "Medialib.h"

#define GETSEL (prox_model->mapSelectionToSource(selectionModel()->selection()).indexes())

MedialibSorted::MedialibSorted(QObject *parent) : QSortFilterProxyModel(parent) {
    //
}

bool MedialibSorted::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    if (left.internalPointer() && right.internalPointer()) {
        ddb_medialib_item_t *l = static_cast<ddb_medialib_item_t *>(left.internalPointer());
        ddb_medialib_item_t *r = static_cast<ddb_medialib_item_t *>(right.internalPointer());
        //do not sort tracks
        if (!l->track && !r->track) {
            return QString(l->text) < QString(r->text);
        }
    }
    return false;
}


MedialibTreeView::MedialibTreeView(QWidget *parent, DBApi *Api) : QTreeView(parent) {
    api = Api;
    // Tree setup
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setHeaderHidden(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    // Model
    ms_model = new MediasourceModel(this,Api,"medialib");
    prox_model = new MedialibSorted(this);
    prox_model->setSourceModel(ms_model);
    setModel(prox_model);
    connect(ms_model,SIGNAL(modelReset()), this, SLOT(onModelReset()));

    // Context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    actions = new QActionGroup(this);
    actions->setExclusive(false);
    QAction *a = actions->addAction(new QAction(tr("Add To Playback Queue")));
    a->setObjectName("add_to_playback_queue");
    connect (a, SIGNAL(triggered()), this, SLOT(onAddToPlaybackQueue()));
    a = actions->addAction(new QAction(tr("Remove From Playback Queue")));
    a->setObjectName("remove_from_playback_queue");
    connect (a, SIGNAL(triggered()), this, SLOT(onRemoveFromPlaybackQueue()));
    setProperty("Actions",(quintptr) actions);
}

void MedialibTreeView::onModelReset() {
    expand(prox_model->index(0,0,QModelIndex()));
    sortByColumn(0,Qt::AscendingOrder);
}

void MedialibTreeView::showContextMenu(QPoint p) {
    QModelIndexList sel = GETSEL;
    // Disable actions if nothing selected
    foreach(QAction *ac, actions->actions()) {
        if (ac->objectName() != "setup-medialib") {
            ac->setEnabled(sel.length());
        }
    }
    api->playItemContextMenu(this,p);
}

void MedialibTreeView::onAddToPlaybackQueue() {
    QModelIndexList sel = GETSEL;
    playItemList l = ms_model->tracks(sel);
    foreach(DB_playItem_t *it, l) {
        DBAPI->playqueue_push(it);
        DBAPI->pl_item_unref(it);
    }
}

void MedialibTreeView::onRemoveFromPlaybackQueue() {
    QModelIndexList sel = GETSEL;
    playItemList l = ms_model->tracks(sel);
    foreach(DB_playItem_t *it, l) {
        DBAPI->playqueue_remove(it);
        DBAPI->pl_item_unref(it);
    }
}

void MedialibTreeView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
    QTreeView::mousePressEvent(event);
}

void MedialibTreeView::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;
    if (selectedIndexes().length() == 0) {
        return;
    }
    QDrag *drag = new QDrag(this);
    QModelIndexList sel = GETSEL;
    QMimeData *mimeData = api->mime_playItems(ms_model->tracks(sel));
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}

Medialib::Medialib(QWidget *parent, DBApi *Api) : DBWidget(parent, Api) {
    // Tree setup
    tree = new MedialibTreeView(this,Api);
    // Layout setup
    setLayout(new QVBoxLayout(this));
    search_layout = new QHBoxLayout(this);
    search_query = new QComboBox(this);
    search_box = new QLineEdit(this);
    search_layout->addWidget(search_query);
    search_layout->addWidget(search_box);
    search_layout->setContentsMargins(0,0,0,0);
    search_layout->setSpacing(0);
    search_layout_widget = new QWidget();
    search_layout_widget->setLayout(search_layout);
    layout()->addWidget(search_layout_widget);
    layout()->addWidget(tree);
    search_box->setPlaceholderText(QString(tr("Search")) + "...");
    connect(search_box, SIGNAL(textChanged(QString)), tree->ms_model, SLOT(setSearchQuery(QString)));
    // Restore folders
    folders = CONFGET("folders",QStringList()).toStringList();
    tree->ms_model->setDirectories(folders);
    // Setup selectors
    search_query->addItems(tree->ms_model->getSelectors());
    search_query_curr = CONFGET("selectorpos",1).toInt();
    search_query->setCurrentIndex(search_query_curr);
    connect(search_query, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectorChanged(int)));
    tree->ms_model->setSelector(search_query_curr);
    search_query->insertSeparator(search_query->count());
    search_query->addItem(QString("Local"));

    // Options
    parent->setContextMenuPolicy(Qt::ActionsContextMenu);
    set_up_folders = new QAction(QIcon::fromTheme("folder-sync"),"Set up medialib folders...");
    set_up_folders->setObjectName("setup-medialib");
    set_up_folders->setEnabled(true);
    connect(set_up_folders,SIGNAL(triggered()),this, SLOT(folderSetupDialog()));
    parent->addAction(set_up_folders);
    tree->actions->addAction(set_up_folders);
}

Medialib::~Medialib() {
    // todo clean up
}

QWidget *Medialib::constructor(QWidget *parent, DBApi *api) {
    DB_plugin_t *medialib = api->deadbeef->plug_get_for_id("medialib");
    if (!medialib) {
        return new QLabel(QString("Medialib not available"));
    }
    else if (medialib->version_major != DDB_MEDIALIB_VERSION_MAJOR || medialib->version_minor != DDB_MEDIALIB_VERSION_MINOR) {
        return new QLabel(QString("Medialib version mismatch (%1.%2, qt gui: %3.%4")
                          .arg(medialib->version_major) .arg (medialib->version_minor)
                          .arg(DDB_MEDIALIB_VERSION_MAJOR) .arg(DDB_MEDIALIB_VERSION_MINOR));
    }
    return new Medialib(parent,api);
}

void Medialib::onSelectorChanged(int sel) {
    if (sel < tree->ms_model->getSelectors().length()) {
        tree->ms_model->setSelector(sel);
        CONFSET("selectorpos",sel);
        search_query_curr = sel;
    }
    else {
        // todo: implement multiple mediasources support
        search_query->setCurrentIndex(search_query_curr);
    }
}

void Medialib::folderSetupDialog() {
    QDialog d(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    d.setMinimumWidth(405);
    d.setWindowTitle("Set up medialib folders...");
    d.setLayout(new QVBoxLayout());

    d.layout()->addWidget(new QLabel("Directories for medialib to scan:"));

    // List
    lwidget = new QListWidget(&d);
    lwidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d.layout()->addWidget(lwidget);
    // restore
    foreach(QString str,CONFGET("folders",QStringList()).toStringList()) {
        QListWidgetItem *item = new QListWidgetItem(str,lwidget);
        item->setFlags (item->flags() | Qt::ItemIsEditable);
    }

    // Hbox / line
    QWidget *hbox_widget = new QWidget(&d);
    QHBoxLayout *hbox = new QHBoxLayout(hbox_widget);
    hbox->setContentsMargins(0,0,0,0);
    d.layout()->addWidget(hbox_widget);

    ledit = new QLineEdit(&d);
    hbox->addWidget(ledit);
    browse = new QPushButton(QIcon::fromTheme("document-open"), tr("Select folder..."));
    plus = new QPushButton(QIcon::fromTheme("list-add"),QIcon::fromTheme("list-add").isNull() ? tr("Add") : "", &d);
    minus = new QPushButton(QIcon::fromTheme("list-remove"),QIcon::fromTheme("list-remove").isNull() ? tr("Remove"): "", &d);
    hbox->addWidget(browse);
    hbox->addWidget(plus);
    hbox->addWidget(minus);
    // connections
    connect(lwidget, SIGNAL(itemChanged(QListWidgetItem*)),this, SLOT(folderSetupDialogItemHandler(QListWidgetItem*)));
    connect(browse,SIGNAL(clicked(bool)), this, SLOT(folderSetupDialogHandler(bool)));
    connect(plus,SIGNAL(clicked(bool)), this, SLOT(folderSetupDialogHandler(bool)));
    connect(minus,SIGNAL(clicked(bool)), this, SLOT(folderSetupDialogHandler(bool)));

    d.exec();
}

void Medialib::folderSetupDialogHandler(bool checked) {
    Q_UNUSED(checked);
    QObject *s = sender();
    if (s == plus) {
        QString str = ledit->text();
        if (str.length() > 0) {
            QListWidgetItem *item = new QListWidgetItem(str,lwidget);
            item->setFlags (item->flags() | Qt::ItemIsEditable);
            folders.append(str);
            CONFSET("folders",folders);
            ledit->setText("");
        }
    }
    else if (s == minus) {
        QList<QListWidgetItem *> list = lwidget->selectedItems();
        QStringList folders = CONFGET("folders",QStringList()).toStringList();
        for (int i = 0; i < list.length(); i++) {
            int row = lwidget->row(list[i]);
            lwidget->takeItem(row);
            folders.takeAt(row);
        }
        CONFSET("folders",folders);
        tree->ms_model->setDirectories(folders);
    }
    else if (s == browse) {
        QFileDialog dialog(lwidget,tr("Select folder..."),ledit->text().length() ? ledit->text() : "");
        dialog.setFileMode(QFileDialog::Directory);

        if(dialog.exec()) {
            QStringList list = dialog.selectedFiles();
            QString str;
            foreach(str,list) {
                QListWidgetItem *item = new QListWidgetItem(str,lwidget);
                item->setFlags (item->flags() | Qt::ItemIsEditable);
            }
        }
    }
}

void Medialib::folderSetupDialogItemHandler(QListWidgetItem *item) {
    // update if edited
    if (item->text().length() == 0) {
        // empty, remove
        lwidget->takeItem(lwidget->row(item));
    }
    // rebuild and update folders
    QStringList list;
    for (int i = 0; i < lwidget->count(); i++) {
        list.append(lwidget->item(i)->text());
    }
    CONFSET("folders",list);
    tree->ms_model->setDirectories(list);
}
