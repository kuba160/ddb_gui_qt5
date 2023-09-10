#ifndef MEDIASOURCEMODEL_H
#define MEDIASOURCEMODEL_H

#include <QAbstractItemModel>
#include <QObject>

#include <deadbeef/deadbeef.h>

enum MSRole{
    Track = Qt::UserRole,
    Tracks,
    IsSelected,
    IsPartiallySelected,
    IsExpanded,
};

class MediasourceModel : public QAbstractItemModel {
    Q_OBJECT
public:
    MediasourceModel(QObject *parent, DB_mediasource_t *ms, QString source_name);
    ~MediasourceModel();

    ddb_mediasource_source_t* getSource();

    void setScriptable(ddb_scriptable_item_t *s);


    static void source_listener(ddb_mediasource_event_type_t event, void *user_data);
//    playItemList tracks(QModelIndexList &l);
//    playItemList tracks(ddb_medialib_item_t *);
//    QModelIndex indexByPath(QStringList &l);


//    void currentStateClean(CurrentState_t *cs);

    // Model
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QModelIndex parent(const QModelIndex &index) const override;

signals:
    void mediasourceNeedsToreload();

protected:
    DB_mediasource_t *ms = nullptr;
    ddb_mediasource_source_t *source;
    ddb_medialib_item_t *root = nullptr;

    ddb_scriptable_item_t *m_script = nullptr;
    QString filter;
    int listener;

    QHash<int, QByteArray> roleNames() const override;

    static constexpr ddb_medialib_item_t* toMedialibItem(const QModelIndex &index) {
        if(index.isValid() && index.constInternalPointer()) {
            return static_cast<ddb_medialib_item_t *>(index.internalPointer());
        }
        return nullptr;
    }

    constexpr int itemRow(const QModelIndex &index) const {
        int i = 0;
        ddb_medialib_item_t *it = toMedialibItem(index);
        if (it) {
            ddb_medialib_item_t *parent = ms->get_tree_item_parent(it);
            if (parent) {
                const ddb_medialib_item_t *child = ms->tree_item_get_children(parent);
                while (child != it || !child) {
                    child = ms->tree_item_get_next(child);
                    i++;
                }
            }
        }
        return i;
    }

    void recreateTree();

};

#endif // MEDIASOURCEMODEL_H
