#include <QTreeWidget>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <cstdint>

// used for sleep()
#include <unistd.h>

#include "Medialib.h"

QStringList default_query = {"Album", "Artist", "Genre", "Folder"};

static void listener_callback(ddb_mediasource_event_type_t event, void *user_data) {
    Q_UNUSED(event); Q_UNUSED(user_data)
    static_cast<Medialib *>(user_data)->updateTree();
}

void MedialibTreeWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton)
        dragStartPosition = event->pos();
    QTreeWidget::mousePressEvent(event);
}

void MedialibTreeWidget::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength()
         < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QList<DB_playItem_t *> list = static_cast<MedialibTreeWidgetItem *>(selectedItems().at(0))->getTracks();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    stream << (playItemList) {list.count(),list};
    mimeData->setData("medialib/tracks",encodedData);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction | Qt::MoveAction);
}

MedialibTreeWidgetItem::MedialibTreeWidgetItem(QWidget *parent, DBApi *api, ddb_medialib_item_t *it) : DBWidget(parent,api) {
    setText(0,QString(it->text));
    track = it->track;
    ddb_medialib_item_t *child = it ? it->children : nullptr;
    while(child != nullptr) {
        this->addChild(new MedialibTreeWidgetItem(parent, api, child));
        child = child->next;
    }
}

QList<DB_playItem_t *> MedialibTreeWidgetItem::getTracks() {
    QList<DB_playItem_t *> list;
    if (track) {
        list.append(track);
    }
    int i;
    for (i = 0; i < childCount(); i++) {
        list.append(static_cast<MedialibTreeWidgetItem *>(child(i))->getTracks());
    }
    return list;
}

Medialib::Medialib(QWidget *parent, DBApi *Api) : DBWidget(parent, Api) {
    // GUI
    tree = new MedialibTreeWidget();
    tree->setHeaderHidden(true);
    tree->setDragDropMode(QAbstractItemView::DragDrop);
    tree->setDragEnabled(true);
    tree->viewport()->setAcceptDrops(true);
    main_layout = new QVBoxLayout(this);
    search_layout = new QHBoxLayout();
    search_query = new QComboBox();
    search_box = new QLineEdit();
    search_layout->addWidget(search_query);
    search_layout->addWidget(search_box);
    search_layout->setContentsMargins(0,0,0,0);
    search_layout->setSpacing(0);
    main_layout->setContentsMargins(2,4,2,0);
    //main_layout->setSpacing(5);
    main_layout->addLayout(search_layout);
    main_layout->addWidget(tree);
    int i;
    for (i = 0; i < default_query.count(); i++) {
        search_query->addItem(_(default_query.at(i).toUtf8()));
    }
    search_query->setCurrentIndex(1);
    search_query->insertSeparator(search_query->count());
    search_query->addItem(QString("Local"));
    search_box->setPlaceholderText(QString(_("Search")) + "...");
    connect(search_query, SIGNAL(currentIndexChanged(int)), this, SLOT(searchQueryChanged(int)));
    connect(search_box, SIGNAL(textChanged(const QString &)), this, SLOT(searchBoxChanged(const QString &)));

    // DEADBEEF
    DB_plugin_t *medialib = api->deadbeef->plug_get_for_id("medialib");
    ml = static_cast<DB_mediasource_t *>((void *)medialib);
    ml_source = static_cast<ddb_medialib_plugin_t *>((void *)medialib);

    pl_mediasource = ml->create_source("ddb_gui_qt5");
    // medialib thread unsafe at time of programming, wait for it to calm down :)
    sleep(1);

    listener_id = ml->add_listener(pl_mediasource,listener_callback,this);
    // TODO, allow setting folders
    const char *folders[] = {"/media/kuba-kubuntu/Archiwum/Muzyka/", "A:/Muzyka/FLAC", nullptr};
    ml_source->set_folders(pl_mediasource,folders,2);

    // remove after sleep fix
    updateTree();
}

Medialib::~Medialib() {
    ml->remove_listener(pl_mediasource,listener_id);
    if (pl_mediasource)
        ml->free_source(pl_mediasource);
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

void Medialib::updateTree() {
    int index = search_query->currentIndex();
    QString text = search_box->text();
    if (curr_it) {
        ml->free_list(pl_mediasource, curr_it);
    }
    curr_it = ml->create_list (pl_mediasource,
                                               default_query.at(index).toLower().toUtf8(),
                                               text.length() ? text.toUtf8() : " ");
    tree->clear();
    QList<QTreeWidgetItem *> items;
    items.append (new MedialibTreeWidgetItem(this, api, curr_it));

    tree->insertTopLevelItems(0, items);
    tree->expandItem(tree->itemAt(0,0));
    if (tree->itemAt(0,0))
        tree->itemAt(0,0)->sortChildren(0,Qt::AscendingOrder);
}

void Medialib::searchQueryChanged(int index) {
    Q_UNUSED(index);
    updateTree();
}

void Medialib::searchBoxChanged(const QString &text) {
    Q_UNUSED(text);
    updateTree();
}
