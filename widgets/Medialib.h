#ifndef MEDIALIB_H
#define MEDIALIB_H

#include <QSlider>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>

#include <QListWidget>
#include <QPushButton>
#include <QActionGroup>
#include <QFutureWatcher>

#include <deadbeef/deadbeef.h>
#include "DBApi.h"
#include "medialib.h"

#include "../MediasourceModel.h"
#include <QSortFilterProxyModel>

class MedialibSorted : public QSortFilterProxyModel {
    Q_OBJECT
public:
    MedialibSorted (QObject *parent = nullptr);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

// MedialibTreeWidget
class MedialibTreeView : public QTreeView {
    Q_OBJECT
public:
    MedialibTreeView(QWidget *parent = nullptr, DBApi *Api = nullptr);

    QActionGroup *actions;
    MediasourceModel *ms_model = nullptr;
    MedialibSorted *prox_model = nullptr;
protected:
    DBApi *api = nullptr;
    QPoint dragStartPosition;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
public slots:
    void showContextMenu(QPoint p);
    void onAddToPlaybackQueue();
    void onRemoveFromPlaybackQueue();

    void onModelReset();
};

// Medialib
class Medialib : public QWidget, public DBWidget {
    Q_OBJECT

public:
    Medialib(QWidget *parent = nullptr, DBApi *api = nullptr);
    ~Medialib();
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);

public slots:
    void folderSetupDialog();
    void folderSetupDialogHandler(bool checked);
    void folderSetupDialogItemHandler(QListWidgetItem *item);

    void onSelectorChanged(int sel);
private:
    // Widget
    QHBoxLayout *search_layout = nullptr;
    QWidget *search_layout_widget;
    MedialibTreeView *tree = nullptr;
    QComboBox *search_query = nullptr;
    int search_query_curr = 0;
    QLineEdit *search_box = nullptr;

    // Current selection
    QStringList curr_item;

    // Action
    QStringList folders;
    QAction *set_up_folders= nullptr;

    // Dialog
    QListWidget *lwidget = nullptr;
    QLineEdit * ledit = nullptr;
    QObject *filedialog = nullptr;
    QPushButton *browse = nullptr;
    QPushButton *plus = nullptr;
    QPushButton *minus = nullptr;

};

#endif // MEDIALIB_H
