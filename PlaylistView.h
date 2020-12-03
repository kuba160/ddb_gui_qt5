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
#include "DBApi.h"
#include "PlaylistModel.h"

#include <QTreeView>

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

private slots:
    void trackDoubleClicked(QModelIndex index);
    void headerContextMenuRequested(QPoint);
    void lockColumns(bool);
    void lockPlaylist(bool);
    void onTrackChanged(DB_playItem_t *, DB_playItem_t *);
    void showContextMenu(QPoint);
    void saveHeaderState();

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
