#include <QTreeWidget>
#include <QLabel>

// temp
#include <unistd.h>

#include "Medialib.h"

Medialib::Medialib(QWidget *parent, DBApi *Api) : DBWidget(parent, Api) {
    DB_plugin_t *medialib = api->deadbeef->plug_get_for_id("medialib");
    ml = static_cast<DB_mediasource_t *>((void *)medialib);
    ml_source = static_cast<ddb_medialib_plugin_t *>((void *)medialib);

    ddb_mediasource_source_t s = ml->create_source("ddb_gui_qt5");

    // TODO, allow setting folders
    const char *folders[] = {"/media/kuba-kubuntu/Hauptdisk/Archiwum/Muzyka/FLAC/OWN/", nullptr};
    ml_source->set_folders(s,folders,1);

    // medialib thread unsafe at time of programming, wait for it to calm down :)
    sleep(1);

    // TODO choose query with search_query + search_box
    ddb_medialib_item_t *it = ml->create_list(s,"artist",nullptr);

    QList<QTreeWidgetItem *> items;
    items.append (new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList(QString(it->text))));
    FillChildrens(items.at(0),it);

    tree = new QTreeWidget();
    tree->insertTopLevelItems(0, items);
    tree->setHeaderHidden(true);
    main_layout = new QVBoxLayout(this);
    search_layout = new QHBoxLayout();
    search_query = new QComboBox();
    search_box = new QLineEdit();
    search_layout->addWidget(search_query);
    search_layout->addWidget(search_box);
    search_layout->setContentsMargins(0,0,0,0);
    search_layout->setSpacing(0);
    main_layout->setContentsMargins(0,4,0,0);
    //main_layout->setSpacing(5);
    main_layout->addLayout(search_layout);
    main_layout->addWidget(tree);
    search_query->addItem(QString("Artist"));
}

void Medialib::FillChildrens (QTreeWidgetItem *item, ddb_medialib_item_t *it) {
    int i = 0;
    ddb_medialib_item_t *child = it->children;
    while(child != nullptr) {
        item->addChild(new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList(QString(child->text))));
        // TODO missing track pointer for items that should have it
        FillChildrens(item->child(i), child);
        i++;
        if (child->next) {
            child = child->next;
        }
    }
}

QWidget *Medialib::constructor(QWidget *parent, DBApi *api) {
    // check if medialib plugin is available
    DB_plugin_t *medialib = api->deadbeef->plug_get_for_id("medialib");
    if (!medialib) {
        return new QLabel(QString("Medialib not available"));
    }
    return new Medialib(parent,api);
}
/*
void Medialib::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event)
    //tree->resize(this->width(),this->height());
}
*/
