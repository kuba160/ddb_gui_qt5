#include <QTreeWidget>
#include <QLabel>

// used for sleep()
#include <unistd.h>

#include "Medialib.h"

QStringList default_query = {"Album", "Artist", "Genre", "Folder"};

Medialib::Medialib(QWidget *parent, DBApi *Api) : DBWidget(parent, Api) {
    DB_plugin_t *medialib = api->deadbeef->plug_get_for_id("medialib");
    ml = static_cast<DB_mediasource_t *>((void *)medialib);
    ml_source = static_cast<ddb_medialib_plugin_t *>((void *)medialib);

    pl_mediasource = ml->create_source("ddb_gui_qt5");

    // TODO, allow setting folders
    const char *folders[] = {"/media/kuba-kubuntu/Hauptdisk/Archiwum/Muzyka/FLAC/OWN/", "A:/Muzyka/FLAC", nullptr};
    ml_source->set_folders(pl_mediasource,folders,2);

    // medialib thread unsafe at time of programming, wait for it to calm down :)
    sleep(1);

    tree = new QTreeWidget();
    tree->setHeaderHidden(true);
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
    search_query->addItems(default_query);
    search_query->setCurrentIndex(1);
    search_query->insertSeparator(search_query->count());
    search_query->addItem(QString("Local"));
    search_box->setPlaceholderText(QString(_("Search")) + "...");
    connect(search_query, SIGNAL(currentIndexChanged(int)), this, SLOT(searchQueryChanged(int)));
    connect(search_box, SIGNAL(textChanged(const QString &)), this, SLOT(searchBoxChanged(const QString &)));
    updateTree();
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

/*
void Medialib::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event)
    //tree->resize(this->width(),this->height());
}
*/

void Medialib::updateTree() {
    int index = search_query->currentIndex();
    QString text = search_box->text();
    ddb_medialib_item_t *it = ml->create_list (pl_mediasource,
                                               default_query.at(index).toLower().toUtf8(),
                                               text.length() ? text.toUtf8() : " ");
    tree->clear();
    QList<QTreeWidgetItem *> items;
    items.append (new MedialibTreeWidgetItem(this, api, it));
    ml->free_list(pl_mediasource, it);

    tree->insertTopLevelItems(0, items);
    tree->expandItem(tree->itemAt(0,0));
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
