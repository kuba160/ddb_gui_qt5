#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QMimeData>

#include "DBApi.h"

class PlayListModel : public QAbstractItemModel {
    Q_OBJECT
    
public:
    PlayListModel(QObject *parent = nullptr);
    
    void loadConfig();
    void saveConfig();
    
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;

    void insertByURLAtPosition(const QUrl &url, int position = -1);
    void moveItems(QList<int> indices, int before);
    
    void clearPlayList();
    void deleteTracks(const QModelIndexList &tracks);

    QStringList columns;
    QHash<QString, QString> columnNames;

private:
    QVariant data(const QModelIndex &index, int role) const;
    
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    
    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    
    QModelIndex parent(const QModelIndex &child) const;
    
    Qt::DropActions supportedDropActions () const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

protected:
    int status_column;

    QIcon playIcon;
    QIcon pauseIcon;

public Q_SLOTS:
    void trackChanged(DB_playItem_t *from, DB_playItem_t *to);
    void playerPaused();
};

#endif // PLAYLISTMODEL_H
