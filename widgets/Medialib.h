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

// medialib.h
#define DDB_MEDIALIB_VERSION_MAJOR 1
#define DDB_MEDIALIB_VERSION_MINOR 0

typedef struct ddb_medialib_plugin_s {
    DB_mediasource_t plugin;

    unsigned (*folder_count)(ddb_mediasource_source_t source);
    void (*folder_at_index)(ddb_mediasource_source_t source, int index, char *folder, size_t size);
    void (*set_folders) (ddb_mediasource_source_t source, const char **folders, size_t count);
} ddb_medialib_plugin_t;
////

// MedialibTreeWidgetItem
class MedialibTreeWidgetItem : public QObject, public QTreeWidgetItem, public DBWidget {
    Q_OBJECT
public:
    MedialibTreeWidgetItem (QWidget *parent = nullptr, DBApi *api = nullptr, ddb_medialib_item_t *it = nullptr);
    ~MedialibTreeWidgetItem();
    playItemList getTracks();

protected:
    DB_playItem_t *track = nullptr;
    QImage *cover = nullptr;


    QFutureWatcher<QImage *> *cover_watcher = nullptr;
private slots:
    void onCoverLoaded();
};

// MedialibTreeWidget
class MedialibTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    MedialibTreeWidget(QWidget *parent = nullptr, DBApi *Api = nullptr);

    QActionGroup *actions;
protected:
    DBApi *api = nullptr;
    QPoint dragStartPosition;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
public slots:
    void showContextMenu(QPoint p);
    void onAddToPlaybackQueue();
    void onRemoveFromPlaybackQueue();
};

// Medialib
class Medialib : public QWidget, public DBWidget {
    Q_OBJECT

public:
    Medialib(QWidget *parent = nullptr, DBApi *api = nullptr);
    ~Medialib();
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);

    void updateTree();

    void setFolders(QStringList *strlist);

public slots:
    void searchQueryChanged(int index);
    void searchBoxChanged(const QString &text);
    void folderSetupDialog();
    void folderSetupDialogHandler(bool checked);
    void folderSetupDialogItemHandler(QListWidgetItem *item);
private:
    // DeaDBeeF
    ddb_mediasource_source_t pl_mediasource;
    DB_mediasource_t *ml = nullptr;
    ddb_medialib_plugin_t *ml_source = nullptr;
    ddb_mediasource_list_selector_t *ml_selector;
    int listener_id = -1;
    ddb_medialib_item_t *curr_it = nullptr;

    // Widget
    QHBoxLayout *search_layout = nullptr;
    QWidget *search_layout_widget;
    MedialibTreeWidget *tree = nullptr;
    QComboBox *search_query = nullptr;
    int search_query_count = 0;
    QLineEdit *search_box = nullptr;

    // Action
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
