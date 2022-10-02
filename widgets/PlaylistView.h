#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <QStandardItemModel>
#include <QDropEvent>
#include <QUrl>
#include <QMenu>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMimeData>

#include <dbapi/DBApi.h>

#include <QTreeView>
#include <QStyledItemDelegate>
#include <QIdentityProxyModel>

class AutoToolTipDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    AutoToolTipDelegate(QObject* parent);
public slots:
    bool helpEvent(QHelpEvent* e, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index);
};

class PlayingStatusProxyModel : public QIdentityProxyModel {
    QIcon playing;
    QIcon paused;
public:
    PlayingStatusProxyModel(QObject *parent, QAbstractItemModel *model);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};

class PlaylistView : public QTreeView {
    Q_OBJECT
    DBApi *api;
    QString internalName;
    PlayingStatusProxyModel *m;
    PlayItemTableProxyModel *roletable;

public:
    PlaylistView(QWidget *parent, DBApi *Api, QString internalName);
    ~PlaylistView();

    void setModel(QAbstractItemModel *) override;


    // Actions
//    QAction *add_to_playback_queue;
//    QAction *remove_from_playback_queue;
//    QAction *cut;
//    QAction *copy;
//    QAction *paste;
//    QAction *delete_action;

    // Menu position
//    QPoint menu_pos;

    //
//    QMenu headerContextMenu;
//    QList<QAction*> headerActions;
//    QMenu *headerGrouping;
    // header num for current header menu
//    int headerMenu_pos;


    void headerContextMenuRequested(QPoint pos);

};

class HeaderDialog : public QDialog {
    Q_OBJECT
public:
    HeaderDialog(PlayItemTableProxyModel *table, QWidget *parent = nullptr, int headernum = -1);

    QFormLayout layout;
    QDialogButtonBox buttons;
    QLineEdit title;
    QComboBox type;
    QWidget format_parent;
    QHBoxLayout format_layout;
    QLineEdit format;
    QString format_saved;
    QLabel format_help;

protected:
    int header_idx;
    PlayItemTableProxyModel *table;

private slots:
    void onTypeChanged(int index);
    void onAccepted();
    void onRejected();
};

class PlayItemSelectionModel : public QItemSelectionModel {
    QAbstractItemModel *model;
    PlayItemSelectionModel(QAbstractItemModel *model);
};

#endif // PLAYLISTVIEW_H
