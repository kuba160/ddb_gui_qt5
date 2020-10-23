
#ifndef MEDIALIB_H
#define MEDIALIB_H

#include <QSlider>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <deadbeef/deadbeef.h>
#include "DBApi.h"

// medialib.h
#define DDB_MEDIALIB_VERSION_MAJOR 1
#define DDB_MEDIALIB_VERSION_MINOR 0

typedef struct ddb_medialib_plugin_s {
    DB_mediasource_t plugin;
#pragma mark - Configuration

    unsigned (*folder_count)(ddb_mediasource_source_t source);
    void (*folder_at_index)(ddb_mediasource_source_t source, int index, char *folder, size_t size);
    void (*set_folders) (ddb_mediasource_source_t source, const char **folders, size_t count);
} ddb_medialib_plugin_t;
////

class Medialib : public QWidget, public DBWidget {
    Q_OBJECT

public:
    Medialib(QWidget *parent = nullptr, DBApi *api = nullptr);
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);

protected:
    //virtual void resizeEvent(QResizeEvent *event);
private:
    QTreeWidget *tree = nullptr;
    DB_mediasource_t *ml = nullptr;
    ddb_medialib_plugin_t *ml_source = nullptr;

    void FillChildrens (QTreeWidgetItem *item, ddb_medialib_item_t *it);


    QHBoxLayout *search_layout = nullptr;
    QVBoxLayout *main_layout = nullptr;
    QComboBox *search_query = nullptr;
    QLineEdit *search_box = nullptr;

};

#endif // VOLUMESLIDER_H
