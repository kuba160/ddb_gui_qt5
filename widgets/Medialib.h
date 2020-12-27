#ifndef MEDIALIB_H
#define MEDIALIB_H

#include <QSlider>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>

#include <QListWidget>
#include <QPushButton>

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
class MedialibTreeWidgetItem : public QTreeWidgetItem, public DBWidget {
public:
    MedialibTreeWidgetItem (QWidget *parent = nullptr, DBApi *api = nullptr, ddb_medialib_item_t *it = nullptr);
    QList<DB_playItem_t *> getTracks();
    DB_playItem_t *track = nullptr;
};

// MedialibTreeWidget
class MedialibTreeWidget : public QTreeWidget {
protected:
    QPoint dragStartPosition;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
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
    QTreeWidget *tree = nullptr;
    QComboBox *search_query = nullptr;
    int search_query_count = 0;
    QLineEdit *search_box = nullptr;

    // Action
    QAction *set_up_folders= nullptr;

    // Dialog
    QListWidget *lwidget = nullptr;
    QLineEdit * ledit = nullptr;
    QPushButton *plus = nullptr;
    QPushButton *minus = nullptr;
};

#endif // MEDIALIB_H
