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
#include <cstdint>

// used for sleep()
#include <unistd.h>

#include "Medialib.h"


QStringList default_query = {"Album", "Artist", "Genre", "Folder"};

static void listener_callback(ddb_mediasource_event_type_t event, void *user_data) {
    Q_UNUSED(event); Q_UNUSED(user_data)
    qDebug() <<"Callback";
    //static_cast<Medialib *>(user_data)->updateTree();
}

MedialibTreeWidget::MedialibTreeWidget(QWidget *parent, DBApi *Api) : QTreeWidget(parent) {
    api = Api;
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
    if (selectedItems().length() == 0) {
        return;
    }
    QDrag *drag = new QDrag(this);
    QList<DB_playItem_t *> list;
    for (int i = 0; i < selectedItems().length(); i++) {
        list.append(static_cast<MedialibTreeWidgetItem *>(selectedItems().at(i))->getTracks());
    }
    // clone

    QList<DB_playItem_t *> list_copy;
    DB_playItem_t *it;
    foreach(it,list) {
        DB_playItem_t *it_new = DBAPI->pl_item_alloc();
        DBAPI->pl_item_copy(it_new,it);
        //DBAPI->pl_item_unref(it_new);
        list_copy.append(it_new);
    }
    QMimeData *mimeData = api->mime_playItems(list_copy);
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
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
    tree = new MedialibTreeWidget(this,Api);
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->setHeaderHidden(true);
    tree->setDragDropMode(QAbstractItemView::DragDrop);
    tree->setDragEnabled(true);
    tree->viewport()->setAcceptDrops(true);
    setLayout(new QVBoxLayout());
    search_layout = new QHBoxLayout();
    search_query = new QComboBox(this);
    search_box = new QLineEdit(this);
    search_layout->addWidget(search_query);
    search_layout->addWidget(search_box);
    search_layout->setContentsMargins(0,0,0,0);
    search_layout->setSpacing(0);
    search_layout_widget = new QWidget();
    search_layout_widget->setLayout(search_layout);
    this->layout()->addWidget(search_layout_widget);
    this->layout()->addWidget(tree);
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

    QStringList folders = CONFGET("folders",QStringList()).toStringList();
    setFolders(&folders);
    // selectors
    ml_selector = ml->get_selectors(pl_mediasource);
    const char* selector;
    while ((selector = ml->get_name_for_selector(pl_mediasource,ml_selector[search_query_count]))) {
        search_query->addItem(_(selector));
        search_query_count++;
    }
    search_query->setCurrentIndex(1);
    search_query->insertSeparator(search_query->count());
    search_query->addItem(QString("Local"));
    // remove after sleep fix
    updateTree();

    // Options
    parent->setContextMenuPolicy(Qt::ActionsContextMenu);
    set_up_folders = new QAction(QIcon::fromTheme("document-properties"),"Set up medialib folders...");
    connect(set_up_folders,SIGNAL(triggered()),this, SLOT(folderSetupDialog()));
    parent->addAction(set_up_folders);
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
                               ml_selector[index],
                               text.length() ? text.toUtf8() : " ");
    if (!curr_it) {
        qWarning() << "qtMedialib: Tried to updateTree, but medialib failed" << ENDL;
        return;
    }
    tree->clear();
    QList<QTreeWidgetItem *> items;
    items.append (new MedialibTreeWidgetItem(this, api, curr_it));

    tree->insertTopLevelItems(0, items);
    if (tree->itemAt(0,0)) {
        tree->expandItem(tree->itemAt(0,0));
        tree->itemAt(0,0)->sortChildren(0,Qt::AscendingOrder);
    }
}

void Medialib::searchQueryChanged(int index) {
    Q_UNUSED(index);
    if (index < search_query_count) {
        updateTree();
    }
    else if (search_query_count !=0){
        qDebug() << "qtMedialib: backend change" << ENDL;
        search_query->setCurrentIndex(1);
        //updateTree();
    }
}

void Medialib::searchBoxChanged(const QString &text) {
    Q_UNUSED(text);
    updateTree();
}

void Medialib::setFolders(QStringList *folders) {
    if (folders->length() > 0) {
        const char *arr[folders->length()];
        int i = 0;
        QString str;
        foreach(str, *folders) {
            arr[i] = strdup(str.toUtf8().constData());
            i++;
        }
        ml_source->set_folders(pl_mediasource,arr,folders->length());
        for (i = 0; i < folders->length(); i++) {
            if (arr[i]) {
                free((void *) arr[i]);
            }
        }
        CONFSET("folders",*folders);
    }
    else {
        const char *arr[1] = {nullptr};
        ml_source->set_folders(pl_mediasource,arr,1);
        CONFSET("folders",QStringList());
    }
}

void Medialib::folderSetupDialog() {
    QDialog d(this);
    d.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    d.setMinimumWidth(405);
    d.setWindowTitle("Set up medialib folders...");
    d.setLayout(new QVBoxLayout());

    d.layout()->addWidget(new QLabel("Directories for medialib to scan:"));

    // List
    lwidget = new QListWidget(&d);
    lwidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    d.layout()->addWidget(lwidget);
    // restore
    QString n;
    foreach(n,CONFGET("folders",QStringList()).toStringList()) {
        QListWidgetItem *item = new QListWidgetItem(n,lwidget);
        item->setFlags (item->flags() | Qt::ItemIsEditable);
    }

    // Hbox / line
    QWidget *hbox_widget = new QWidget(&d);
    QHBoxLayout *hbox = new QHBoxLayout(hbox_widget);
    hbox->setContentsMargins(0,0,0,0);
    d.layout()->addWidget(hbox_widget);

    ledit = new QLineEdit(&d);
    hbox->addWidget(ledit);
    // todo translate
    browse = new QPushButton(QIcon::fromTheme(""), "Browse...");
    plus = new QPushButton(QIcon::fromTheme("list-add"),QIcon::fromTheme("list-add").isNull() ? "Add" : "", &d);
    minus = new QPushButton(QIcon::fromTheme("list-remove"),QIcon::fromTheme("list-remove").isNull() ? "Remove": "", &d);
    hbox->addWidget(browse);
    hbox->addWidget(plus);
    hbox->addWidget(minus);
    // connections
    connect(lwidget, SIGNAL(itemChanged(QListWidgetItem *)),this, SLOT(folderSetupDialogItemHandler(QListWidgetItem *)));
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
            ledit->setText("");
            // item gets created and appended onto lwidget, itemhandler will handle it, no need to do it here
            ledit->setPlaceholderText("Restart might be needed to update medialib...");
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
        setFolders(&folders);
    }
    else if (s == browse) {
        QFileDialog dialog(lwidget,_("Select folder..."),ledit->text().length() ? ledit->text() : "");
        dialog.setFileMode(QFileDialog::Directory);
        //dialog.setOption(QFileDialog::ShowDirsOnly, false);

        if(dialog.exec()) {
            QStringList list = dialog.selectedFiles();
            QString str;
            foreach(str,list) {
                QListWidgetItem *item = new QListWidgetItem(str,lwidget);
                item->setFlags (item->flags() | Qt::ItemIsEditable);
            }
            ledit->setPlaceholderText("Restart might be needed to update medialib...");
        }
    }
}

void Medialib::folderSetupDialogItemHandler(QListWidgetItem *item) {
    if (item->text().length() == 0) {
        lwidget->takeItem(lwidget->row(item));
    }
    int i;
    QStringList list;
    for (i = 0; i < lwidget->count(); i++) {
        list.append(lwidget->item(i)->text());
    }
    setFolders(&list);
}
