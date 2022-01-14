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
#include "DBApi.h"
#include "PlaylistModel.h"

#include <QTreeView>
#include <QStyledItemDelegate>

class AutoToolTipDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    AutoToolTipDelegate(QObject* parent);
public slots:
    bool helpEvent(QHelpEvent* e, QAbstractItemView* view, const QStyleOptionViewItem& option,
                   const QModelIndex& index);
};

class PlaylistView : public QTreeView, public DBWidget {
    Q_OBJECT

public:
    PlaylistView(QWidget *parent = nullptr, DBApi *Api = nullptr, PlayItemTableModel *ptm = nullptr);
    ~PlaylistView();

    // cursor
    void restoreCursor();
    void storeCursor();

    void goToLastSelection();

    PlayItemTableModel *pi_model;

protected:
    void startDrag(Qt::DropActions supportedActions);
    void dragMoveEvent(QDragMoveEvent* event);

    // Actions
    QAction *add_to_playback_queue;
    QAction *remove_from_playback_queue;
    QAction *cut;
    QAction *copy;
    QAction *paste;
    QAction *delete_action;

    // Menu position
    QPoint menu_pos;

    //
    QMenu headerContextMenu;
    QList<QAction*> headerActions;
    QMenu *headerGrouping;
    // header num for current header menu
    int headerMenu_pos;



    // Enter event
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    // Enter event
    void enterRelease(QModelIndex);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);


protected slots:
    void headerContextMenuRequested(QPoint);
    void lockColumns(bool);
    void lockPlaylist(bool);
    void onTrackChanged(DB_playItem_t *, DB_playItem_t *);
    void showContextMenu(QPoint);
    void saveHeaderState();

    // Actions
    void onAddToPlaybackQueue();
    void onRemoveFromPlaybackQueue();
    void onCut();
    void onCopy();
    void onPaste();
    void onDelete();

    void onHeaderDialogAdd();
    void onHeaderDialogEdit();
    void onHeaderDialogRemove();

};

class HeaderDialog : public QDialog {
    Q_OBJECT
public:
    HeaderDialog(PlayItemTableModel *pt_model, QWidget *parent = nullptr, int headernum = -1);

    QFormLayout layout;
    QDialogButtonBox buttons;
    QLineEdit title;
    QComboBox type;
    QWidget format_parent;
    QHBoxLayout format_layout;
    QLineEdit format;
    QString format_saved;
    QLabel format_help;
    int n;
    int editting = 0;

    PlayItemTableModel *pt;

private slots:
    void onTypeChanged(int index);
    void onAccepted();
    void onRejected();
};

#endif // PLAYLISTVIEW_H
