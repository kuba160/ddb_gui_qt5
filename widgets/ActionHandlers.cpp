#include "ActionHandlers.h"
#include <QDebug>
#include <QObject>
#include <QFileDialog>
#include <QProgressDialog>
#include <QInputDialog>
#include <QMessageBox>

ActionHandlers::ActionHandlers(QWidget *parent, DBApi *Api) : QObject(parent) {
    api = Api;
    DBAction *action = nullptr;


    if ((action = Api->actions.getAction("q_open_files"))) {
        connect(action, &DBAction::actionApplied, Api,  [parent, Api]() {
            QString caption = tr("Open file(s)...");
            QString last_dir = Api->conf.get("FileChooser", "lastdir", "").toString();
            QStringList files = QFileDialog::getOpenFileNames(parent, caption, last_dir);
            if (files.size()) {
                QFuture<PlayItemIterator> future_import = Api->playlist.runFileImport(files);
                QFutureWatcher<PlayItemIterator> *watcher = new QFutureWatcher<PlayItemIterator>(parent);
                watcher->setFuture(future_import);
                connect(watcher, &QFutureWatcher<PlayItemIterator>::finished, Api, [watcher, Api]() {
                    PlayItemIterator pit = watcher->result();
                    QMimeData *mime = pit.toMimeData();
                    Api->playlist.getCurrentPlaylist()->dropMimeData(mime,Qt::CopyAction,-1,-1,QModelIndex());
                    delete mime;
                    delete watcher;
                });
                QProgressDialog *dialog = new QProgressDialog(parent);
                dialog->setMinimum(watcher->progressMinimum());
                dialog->setMaximum(watcher->progressMaximum());
                connect (watcher, &QFutureWatcher<PlayItemIterator>::progressTextChanged, dialog, [dialog](const QString &text){
                   dialog->setLabelText(text);
                });
                connect (watcher, &QFutureWatcher<PlayItemIterator>::progressValueChanged, dialog, [dialog](int value){
                   dialog->setValue(value);
                });
                connect (dialog, &QProgressDialog::canceled, watcher, [watcher](){
                   watcher->cancel();
                   watcher->waitForFinished();
                });
                dialog->setMinimumDuration(500);
                Api->conf.set("FileChooser", "lastdir", QUrl(files.at(0)).path());
            }
        });
    }
    // todo add files
    if ((action = Api->actions.getAction("q_add_folders"))) {
        connect(action, &DBAction::actionApplied, Api,  [parent, Api]() {
            QString caption = tr("Add folder(s)...");
            QString last_dir = Api->conf.get("FileChooser", "lastdir", "").toString();
            QString folder = QFileDialog::getExistingDirectory(parent, caption, last_dir);
            if (!folder.isNull()) {
                QProgressDialog *dialog = new QProgressDialog(parent);
                QFuture<PlayItemIterator> future_import = Api->playlist.runFolderImport(QStringList{folder});
                QFutureWatcher<PlayItemIterator> *watcher = new QFutureWatcher<PlayItemIterator>(parent);
                watcher->setFuture(future_import);
                connect(watcher, &QFutureWatcher<PlayItemIterator>::finished, Api, [watcher, dialog, Api]() {
                    PlayItemIterator pit = watcher->result();
                    QMimeData *mime = pit.toMimeData();
                    Api->playlist.getCurrentPlaylist()->dropMimeData(mime,Qt::CopyAction,-1,-1,QModelIndex());
                    delete mime;
                    delete watcher;
                    delete dialog;
                });

                dialog->setMinimum(watcher->progressMinimum());
                dialog->setMaximum(0);//watcher->progressMaximum());
                connect (watcher, &QFutureWatcher<PlayItemIterator>::progressTextChanged, dialog, [dialog](const QString &text){
                   dialog->setLabelText(text);
                });
                connect (watcher, &QFutureWatcher<PlayItemIterator>::progressValueChanged, dialog, [dialog](int value){
                   dialog->setValue(0);//value);
                });
                connect (dialog, &QProgressDialog::canceled, watcher, [watcher](){
                   watcher->cancel();
                   //watcher->waitForFinished();
                });
                dialog->setMinimumDuration(500);
                Api->conf.set("FileChooser", "lastdir", QUrl(folder));
            }
        });
    }
    if ((action = Api->actions.getAction("q_add_location"))) {
        connect(action, &DBAction::actionApplied, Api, [parent, Api]() {
            bool success = false;
            QString url = QInputDialog::getText(parent, tr("Enter URL..."), tr("URL: "), QLineEdit::Normal, QString(), &success, Qt::Dialog, Qt::ImhUrlCharactersOnly);
            PlayItemIterator pit = PlayItemIterator(QUrl(url));
            if (pit.contextType() != PlayItemIterator::ContextType::NONE) {
                QMimeData *mime = pit.toMimeData();
                Api->playlist.getCurrentPlaylist()->dropMimeData(mime,Qt::CopyAction,-1,-1,QModelIndex());
                delete mime;
            }
        });
    }
    if ((action = Api->actions.getAction("q_load_playlist"))) {
        connect(action, &DBAction::actionApplied, Api,  [parent, Api]() {
            QString caption = tr("Open file(s)...");
            QString last_dir = Api->conf.get("FileChooser", "lastdir", "").toString();
            QString filters = "Supported playlist formats (*.dbpl);;Other (*.*)";
            QStringList files = QFileDialog::getOpenFileNames(parent, caption, last_dir, filters);
            if (files.size()) {
                QFuture<PlayItemIterator> future_import = Api->playlist.runPlaylistImport(files);
                QFutureWatcher<PlayItemIterator> *watcher = new QFutureWatcher<PlayItemIterator>(parent);
                watcher->setFuture(future_import);
                connect(watcher, &QFutureWatcher<PlayItemIterator>::finished, Api, [watcher, Api]() {
                    PlayItemIterator pit = watcher->result();
                    QMimeData *mime = pit.toMimeData();
                    Api->playlist.getCurrentPlaylist()->dropMimeData(mime,Qt::CopyAction,-1,-1,QModelIndex());
                    delete mime;
                    delete watcher;
                });
                QProgressDialog *dialog = new QProgressDialog(parent);
                dialog->setMinimum(watcher->progressMinimum());
                dialog->setMaximum(watcher->progressMaximum());
                connect (watcher, &QFutureWatcher<PlayItemIterator>::progressTextChanged, dialog, [dialog](const QString &text){
                   dialog->setLabelText(text);
                });
                connect (watcher, &QFutureWatcher<PlayItemIterator>::progressValueChanged, dialog, [dialog](int value){
                   dialog->setValue(value);
                });
                connect (dialog, &QProgressDialog::canceled, watcher, [watcher](){
                   watcher->cancel();
                   watcher->waitForFinished();
                });
                dialog->setMinimumDuration(500);
                Api->conf.set("FileChooser", "lastdir", QUrl(files.at(0)).path());
            }
        });
    }
    if ((action = Api->actions.getAction("q_quit"))) {
        connect(action, &DBAction::actionApplied, Api,  [parent, Api]() {
            parent->close();
        });
    }
    if ((action = Api->actions.getAction("q_about_qt"))) {
        connect(action, &DBAction::actionApplied, Api,  [parent, Api]() {
            QMessageBox::aboutQt(parent);
        });
    }


}
