#include "PlaylistView.h"
#include "dbapi/models/PlayItemModel.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QProxyStyle>
#include <QToolTip>
#include <QDrag>

#include "DBActionMenu.h"

#define DBAPI (this->api)

class myViewStyle: public QProxyStyle{
public:
    myViewStyle(QStyle* style = 0);

    void drawPrimitive ( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget = 0 ) const;
};

myViewStyle::myViewStyle(QStyle* style)
     :QProxyStyle(style)
{}

void myViewStyle::drawPrimitive ( PrimitiveElement element, const QStyleOption * option, QPainter * painter, const QWidget * widget) const{
    if (element == QStyle::PE_IndicatorItemViewItemDrop){
        QStyleOption opt(*option);
        opt.rect.setLeft(0);
        if (widget) {
            opt.rect.setRight(widget->width());
        }
        // TODO fix drawing when pos = -1
        QProxyStyle::drawPrimitive(element, &opt, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}


AutoToolTipDelegate::AutoToolTipDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

bool AutoToolTipDelegate::helpEvent(QHelpEvent* e, QAbstractItemView* view, const QStyleOptionViewItem& option,
                                    const QModelIndex& index) {
    if (!e || !view) {
        return false;
    }
    if (e->type() == QEvent::ToolTip) {
        QRect rect = view->visualRect(index);
#if QT_VERSION > QT_VERSION_CHECK(5,11,0)
        int px = QFontMetrics(view->font()).horizontalAdvance(index.data(Qt::DisplayRole).toString().append(".."));
#else
        int px = QFontMetrics(view->font()).width(index.data(Qt::DisplayRole).toString().append(".."));
#endif
        // calculate identation
        QModelIndex ident_scan = index;
        int par = 0;
        while (ident_scan.parent().isValid()) {
            ident_scan = ident_scan.parent();
            // todo more precise calculation
            par++;
        }
        px += par >= 2 ? 55 : par == 1 ? 62 : 0;
        if (rect.width() < px) {
            QVariant tooltip = index.data(Qt::DisplayRole);
            if (tooltip.canConvert<QString>()) {
                QToolTip::showText(e->globalPos(),tooltip.toString(),view);
                return true;
            }
        }
        if (!QStyledItemDelegate::helpEvent(e, view, option, index)) {
            QToolTip::hideText();
        }
        return true;
    }
    return QStyledItemDelegate::helpEvent(e, view, option, index);
}


PlayingStatusProxyModel::PlayingStatusProxyModel(QObject *parent, QAbstractItemModel *model)
    : QIdentityProxyModel(parent),
      playing(":/images/play_16.png"),
      paused(":/images/pause_16.png") {
    setSourceModel(model);
}

QVariant PlayingStatusProxyModel::data(const QModelIndex &index, int role) const {
    if (role == PlayItemModel::ItemPlayingDecoration) {
        int status = sourceModel()->data(index, PlayItemModel::ItemPlayingState).toInt();
        if (status == 1) {
            return playing;
        }
        else if (status == 2) {
            return paused;
        }
    }
    return sourceModel()->data(index, role);
}

PlaylistView::PlaylistView(QWidget *parent, DBApi *Api, QString name) : QTreeView(parent) {
    api = Api;
    internalName = name;

    setAutoFillBackground(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setDragEnabled(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setIconSize(QSize(16, 16));
    setTextElideMode(Qt::ElideRight);
    setIndentation(0);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setItemsExpandable(false);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setWordWrap(false);
    setExpandsOnDoubleClick(false);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setStyle(new myViewStyle);
    setItemDelegate(new AutoToolTipDelegate(this));

    header()->setStretchLastSection(false);
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setSortIndicatorShown(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    header()->setFirstSectionMovable(true);
#endif

    // header context menu
    connect(header(), &QHeaderView::customContextMenuRequested, this, &PlaylistView::headerContextMenuRequested);

    
    // Context menu (TODO into core)
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, [this](const QPoint &p) {
        ActionContext *context = new ActionContext(true);
        QMenu *menu = DBAPI->actions.buildTrackMenu(this, "widgets_contextmenu", context).value<QMenu*>();
        if (menu) {
            connect (menu, &QMenu::triggered, this, [this](QAction *) {
                qDebug() << "TEST";
            });
            menu->popup(viewport()->mapToGlobal(p));
        }
    });
    //QAction *delTrack = new QAction(tr("Remove Track(s) From Playlist"), this);
    //delTrack->setShortcut(Qt::Key_Delete);
    //connect(delTrack, SIGNAL(triggered()), this, SLOT(delSelectedTracks()));
    //addAction(delTrack);
    //connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

    // Header lock
    bool locked = api->conf.get(internalName,"HeadersLocked", false).toBool();
    header()->setSectionResizeMode(locked ? QHeaderView::Fixed : QHeaderView::Interactive);
    header()->setSectionsMovable(!locked);

    // Set Model
    m = new PlayingStatusProxyModel(this, nullptr);
    roletable = DBAPI->playlist.createTableProxy(this);
    QByteArray headerconfig = DBAPI->conf.get(name, "playlistview_headerconfig", QByteArray()).toByteArray();
    roletable->setHeaderConfiguration(headerconfig);

    roletable->setSourceModel(m);
    header()->restoreState(DBAPI->conf.get(internalName,"playlistview_headerdata", QByteArray()).toByteArray());
    QTreeView::setModel(roletable);
}

void PlaylistView::setModel(QAbstractItemModel *model) {
    m->setSourceModel(model);
}


PlaylistView::~PlaylistView() {
    DBAPI->conf.set(internalName, "playlistview_headerconfig",
                    roletable->getHeaderConfiguration());
    DBAPI->conf.set(internalName, "playlistview_headerdata", header()->saveState());

    // free header actions
    /*
    int i;
    for (i = 0; i < headerActions.length(); i++) {
        delete headerActions.at(i);
    }*/
    // maybe free headers?
}


// HEADER

void PlaylistView::headerContextMenuRequested(QPoint pos) {
    int headerMenu_pos =  header()->logicalIndexAt(pos);

    QMenu *menu = new QMenu(this);
    menu->addAction(QIcon::fromTheme("list-add"), tr("Add column"), this,
                    [this]() {
        HeaderDialog *dialog = new HeaderDialog(roletable, this,-1);
        connect(dialog, &QDialog::finished, this, [this]() {
            DBAPI->conf.set(internalName, "playlistview_headerconfig",
                            roletable->getHeaderConfiguration());
            DBAPI->conf.set(internalName,"playlistview_headerdata", header()->saveState());
        });
        dialog->open();
    });
    menu->addAction(QIcon::fromTheme("edit-entry"), tr("Edit column"), this,
                    [this, headerMenu_pos]() {
        HeaderDialog *dialog = new HeaderDialog(roletable, this, headerMenu_pos);
        connect(dialog, &QDialog::finished, this, [this]() {
            DBAPI->conf.set(internalName, "playlistview_headerconfig",
                            roletable->getHeaderConfiguration());
            DBAPI->conf.set(internalName,"playlistview_headerdata", header()->saveState());
        });
        dialog->open();
    });
    menu->addAction(QIcon::fromTheme("list-remove"), tr("Remove column"), this,
                    [this, headerMenu_pos]() {
        roletable->removeHeader(headerMenu_pos);
        DBAPI->conf.set(internalName, "playlistview_headerconfig",
                        roletable->getHeaderConfiguration());
        DBAPI->conf.set(internalName,"playlistview_headerdata", header()->saveState());
    });

    menu->move(mapToGlobal(pos));
    menu->show();
}

HeaderDialog::HeaderDialog(PlayItemTableProxyModel *table_proxy, QWidget *parent, int headernum) : QDialog(parent) {
    // headernum - column to insert (to be passed in signal)
    // header - pointer if modifying, otherwise null
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    setMinimumWidth(405);
    setAttribute(Qt::WA_DeleteOnClose);

    table = table_proxy;
    header_idx = headernum;

    setWindowTitle(headernum == -1 ? tr("Add column") : tr("Edit column"));


    // layout
    this->setLayout(&layout);
    layout.addRow(tr("Title:"), &title);
    layout.addRow(tr("Type:"), &type);
    QStringList items;
    for (int i = PlayItemModel::ItemIndex; i <= PlayItemModel::PlayItemRoleLast; i++) {
        if (i == PlayItemModel::ItemPlaying) {
            items.append(tr("Playing"));
        }
        else {
            items.append(table->getDefaultHeaderData(i).title);
        }
    }
    items.append(tr("Custom"));

    type.addItems(items);
    //format.setEnabled(false);
    format_parent.setLayout(&format_layout);
    format_layout.addWidget(&format);
    format_layout.addWidget(&format_help);
    format_help.setText(QString("<a href=\"https://github.com/DeaDBeeF-Player/deadbeef/wiki/Title-formatting-2.0\">%1</a>").arg(tr("Help")));
    format_help.setTextFormat(Qt::RichText);
    format_help.setTextInteractionFlags(Qt::TextBrowserInteraction);
    format_help.setOpenExternalLinks(true);
    layout.addRow(tr("Format:"), &format_parent);

    // Missing sort formatting, alignment, color

    if (header_idx == -1) {
        // Add
        type.setCurrentIndex(0);
        title.setPlaceholderText(PlayItemModel::defaultTitle(PlayItemModel::ItemIndex));
    }
    else {
        // Edit
        HeaderData_t hd = table->getHeaderData(header_idx);
        HeaderData_t hd_default = table->getDefaultHeaderData(hd.role);
        // Trim for removing custom roles
        int role = qMin(hd.role,(int) PlayItemModel::PlayItemRoleLast+1);
        type.setCurrentIndex(role - PlayItemModel::ItemIndex);
        title.setText(hd.title);
        title.setPlaceholderText(hd_default.title);
        if (hd.role > PlayItemModel::PlayItemRoleLast) {
            format.setText(hd.format);
            format.setEnabled(true);
        }
        else {
            format.setText(hd_default.format);
            format.setEnabled(false);
        }
    }

    buttons.addButton(QDialogButtonBox::Ok);
    buttons.addButton(QDialogButtonBox::Cancel);
    layout.addRow("",&buttons);

    connect (&type, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
    connect(&buttons, SIGNAL(accepted()), this, SLOT(onAccepted()));
    connect(&buttons, SIGNAL(rejected()), this, SLOT(onRejected()));
}

void HeaderDialog::onTypeChanged(int idx_new) {
    HeaderData_t hd_default = table->getDefaultHeaderData(idx_new + PlayItemModel::ItemIndex);
    if (idx_new + PlayItemModel::ItemIndex > PlayItemModel::PlayItemRoleLast) {
        format.setEnabled(true);
    }
    else {
        format.setEnabled(false);
        HeaderData_t hd_default = table->getDefaultHeaderData(idx_new + PlayItemModel::ItemIndex);
        format.setText(hd_default.format);
    }
    title.setPlaceholderText(hd_default.title);
}

void HeaderDialog::onAccepted() {
    int role = type.currentIndex() + PlayItemModel::ItemIndex;
    const QString title = this->title.text();
    const QString format = this->format.text();
    if (header_idx == -1) {
        // Add
        table->addHeader({role, title, format});
    }
    else {
        // Edit
        table->replaceHeader(header_idx, {role, title, format});
    }
    close();
}

void HeaderDialog::onRejected() {
    close();
}

PlayItemSelectionModel::PlayItemSelectionModel(QAbstractItemModel *model) {
    this->model = model;

}

//void Playlist::onSelectionChanged() {
//    // update selection to represent deadbeef selection status
//    QItemSelection sel;
//    int count = pi_model->rowCount();
//    for (int i = 0; i < count ; i++) {
//        DB_playItem_t *it = DBAPI->pl_get_for_idx(i);
//        if (it) {
//            if (DBAPI->pl_is_selected(it)) {
//                QModelIndex it_sel = pi_model->index(i,0,QModelIndex());
//                sel.select(it_sel,it_sel);
//            }
//            DBAPI->pl_item_unref(it);
//        }
//    }
//    selectionModel()->select(sel,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
//}

//void Playlist::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
//    // update deadbeef selection status to match gui selection
//    QList<QModelIndex> a = selected.indexes();
//    QVector<int> select_int;
//    int i;
//    for (i = 0; i < a.length(); i++) {
//        if(!select_int.contains(a[i].row())) {
//            select_int.append(a[i].row());
//        }
//    }
//    for (i = 0; i< select_int.length(); i++) {
//        DB_playItem_t *pl = DBAPI->pl_get_for_idx(select_int[i]);
//        if (pl) {
//            if (i == 0) {
//                DBAPI->pl_set_cursor(PL_MAIN, select_int[i]);
//                DBAPI->conf_set_int(QString("playlist.cursor.%1").arg(DBAPI->plt_get_curr_idx()).toUtf8(), select_int[i]);
//            }
//            DBAPI->pl_set_selected(pl, true);
//            //DBAPI->action_set_playlist(plt);
//            DBAPI->pl_item_unref(pl);
//        }
//    }

//    a = deselected.indexes();
//    select_int.clear();
//    for (i = 0; i < a.length(); i++) {
//        if(!select_int.contains(a[i].row())) {
//            select_int.append(a[i].row());
//        }
//    }
//    for (i = 0; i< select_int.length(); i++) {
//        DB_playItem_t *pl = DBAPI->pl_get_for_idx(select_int[i]);
//        if (pl) {
//            DBAPI->pl_set_selected(pl,false);
//            DBAPI->pl_item_unref(pl);
//        }
//    }
//}
