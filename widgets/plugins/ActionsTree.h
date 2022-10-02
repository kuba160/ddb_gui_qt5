#include <QTreeView>

#include <dbapi/DBApi.h>

class ActionsTree : public QTreeView {
    Q_OBJECT
    DBApi *api;
public:
    ActionsTree(QWidget * parent, DBApi *Api);
    ~ActionsTree();

    static QObject *constructor(QWidget *parent = nullptr, DBApi *Api =nullptr);
};
