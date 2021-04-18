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
    PlaylistView(QWidget *parent = nullptr, DBApi *Api = nullptr);
    ~PlaylistView();

    // cursor
    void restoreCursor();
    void storeCursor();

    void clearPlayList();
    void goToLastSelection();

    void insertByURLAtPosition(const QUrl &url, int position = -1);

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

signals:
    // Enter event
    void enterRelease(QModelIndex);

private:
    // Header
    QList<PlaylistHeader_t *> headers;
    QMenu headerContextMenu;
    QList<QAction*> headerActions;
    QMenu *headerGrouping;
    // header num for current header menu
    int headerMenu_pos;



    // Enter event
    bool eventFilter(QObject *obj, QEvent *event);


protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    PlaylistModel playlistModel;

public slots:;
    void delSelectedTracks();

protected slots:
    void trackDoubleClicked(QModelIndex index);
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

    void headerDialogAdd(bool);
    void headerDialogEdit(bool);
    void headerDialogRemove(bool);

    void headerAdd(int,PlaylistHeader_t *);
    void headerEdit(int,PlaylistHeader_t *);
};

class HeaderDialog : public QDialog {
    Q_OBJECT
public:
    HeaderDialog(QWidget *parent = nullptr, int headernum = -1, PlaylistHeader_t *header = nullptr);

    QFormLayout layout;
    QDialogButtonBox buttons;
    QLineEdit title;
    QComboBox type;
    QWidget format_parent;
    QHBoxLayout format_layout;
    QLineEdit format;
    QString format_saved;
    QLabel format_help;
    PlaylistHeader_t *h = nullptr;
    int n;
    int editting = 0;

signals:
    void headerDialogEvent(int headernum, PlaylistHeader_t *header);
private slots:
    void titleEdited(const QString &text);
    void typeChanged(int index);
    void formatEdited(const QString &text);
    void accepted();
    void rejected();
};

#endif // PLAYLISTVIEW_H
