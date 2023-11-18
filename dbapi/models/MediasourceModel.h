#ifndef MEDIASOURCEMODEL_H
#define MEDIASOURCEMODEL_H

#include <QAbstractItemModel>
#include <QObject>

#include "ScriptableModel.h"
#include <deadbeef/deadbeef.h>

enum MSRole{
    Track = Qt::UserRole,
    Tracks,
    IsSelected,
    IsPartiallySelected,
    IsExpanded,
    HasChildren
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

    Q_PROPERTY(QAbstractItemModel *queries READ getQueries CONSTANT)

    Q_PROPERTY(QStringList presets READ getPresets NOTIFY presetsChanged)
    Q_PROPERTY(int preset_idx READ getPresetIdx WRITE setPresetIdx NOTIFY presetIdxChanged)
    Q_PROPERTY(QString filter READ getFilter WRITE setFilter NOTIFY filterChanged)

//    void currentStateClean(CurrentState_t *cs);

    // Model
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QModelIndex parent(const QModelIndex &index) const override;


    QAbstractItemModel * getQueries() {
        return m_script_model;
    }

    int getPresetIdx() {
        return m_preset_idx;
    }
    void setPresetIdx(int idx);


    QStringList getPresets();

    QString getFilter() {return m_filter;};
    void setFilter(QString filter) {
        if (m_filter.compare(filter)) {
            m_filter = filter;
            emit filterChanged();
        }
    };

signals:
    void mediasourceNeedsToreload();
    void presetsChanged();
    void presetIdxChanged();
    void filterChanged();

protected:
    DB_mediasource_t *ms = nullptr;
    ddb_mediasource_source_t *source;
    ddb_medialib_item_t *root = nullptr;

    ddb_scriptable_item_t *m_script = nullptr;
    ScriptableModel *m_script_model = nullptr;
    int m_preset_idx = 0;

    QString m_filter;
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
