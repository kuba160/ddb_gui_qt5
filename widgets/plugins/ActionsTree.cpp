#include "ActionsTree.h"
#include <dbapi/models/ActionsModel.h>
#define DBAPI (this->api)

ActionsTree::ActionsTree(QWidget *parent, DBApi *Api) : QTreeView(parent) {
    api = Api;
    setModel(DBAPI->actions.getActionsModel());

    connect(this,&QAbstractItemView::doubleClicked, this, [this](const QModelIndex &idx) {
        if (!model()->hasChildren(idx))
            model()->setData(idx, {}, ActionsModel::ACTION_EXECUTE_MAIN);
    });

}

ActionsTree::~ActionsTree() {
}

QObject * ActionsTree::constructor(QWidget *parent, DBApi *Api) {
    if (!parent) {
        QObject *info = new QObject(nullptr);
        info->setProperty("friendlyName", info->tr("Actions Tree"));
        info->setProperty("internalName", "actionsTree");
        info->setProperty("widgetType", "main");
        info->setProperty("widgetStyle", "Qt Widgets");
        return info;
    }
    return new ActionsTree(parent, Api);
}
