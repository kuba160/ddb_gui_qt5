#include "QueueManager.h"
#include "../PlaylistView.h"
#include <dbapi/models/PlayItemModel.h>


QObject * QueueManager::constructor(QWidget *parent, DBApi *Api) {
    if (!parent) {
        QObject *info = new QObject(nullptr);
        info->setProperty("friendlyName", info->tr("Queue Manager"));
        info->setProperty("internalName", "queueManager");
        info->setProperty("widgetType", "main");
        info->setProperty("widgetStyle", "Qt Widgets");
        return info;
    }

    PlaylistView *widget = new PlaylistView(parent, Api, parent->property("internalName").toString());

    widget->setModel(Api->playlist.getQueue());
    //widget->model()->setHeaderData(0,Qt::Horizontal, PlayItemModel::ItemNumber, Qt::DisplayRole);
    //widget->model()->setHeaderData(0,Qt::Horizontal, PlayItemModel::ItemEmpty, Qt::DecorationRole);
    //connect(widget,&QAbstractItemView::doubleClicked, widget, [widget,Api](const QModelIndex &idx) {Api->playback.play(idx.row());});
    return widget;
}
