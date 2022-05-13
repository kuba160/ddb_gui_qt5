#include "DBFileDialog.h"

#include <QUrl>

#include "QtGui.h"
#include "MainWindow.h"
#undef DBAPI
#define DBAPI deadbeef_internal

DBFileDialog::DBFileDialog(QWidget *parent, DialogType type) : QFileDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);
    m_type = type;
    const char *conf_str;
    QFileDialog::FileMode mode;
    QFileDialog::Options options;
    QStringList filters;

    switch (type) {
        case LOAD_PLAYLIST:
        case SAVE_PLAYLIST:
            conf_str = "filechooser.playlist.lastdir";
            break;
        default:
            conf_str = "filechooser.lastdir";
    }

    switch (type) {
        case OPEN_FILES:
        case ADD_FILES:
            mode = QFileDialog::ExistingFiles;
            options = options | QFileDialog::ReadOnly;
            break;
        case LOAD_PLAYLIST:
            mode = QFileDialog::ExistingFile;
            options = options | QFileDialog::ReadOnly;
            filters << tr("Supported playlist formats (*.dbpl)");
            filters << tr("Other files (*)");
            break;
        case ADD_FOLDERS:
            mode = QFileDialog::Directory;
            options = options | QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly;
            break;
        case SAVE_PLAYLIST:
            mode = QFileDialog::AnyFile;
            break;
    }

    switch (type) {
        case OPEN_FILES:
            setWindowTitle(tr("Open file(s)..."));
            break;
        case ADD_FILES:
            setWindowTitle(tr("Add file(s) to playlist..."));
            break;
        case ADD_FOLDERS:
            setWindowTitle(tr("Add folder(s) to playlist..."));
            break;
        case LOAD_PLAYLIST:
            setWindowTitle(tr("Load playlist"));
            break;
        case SAVE_PLAYLIST:
            setWindowTitle(tr("Save playlist as..."));
            break;
    }

    if (type == SAVE_PLAYLIST) {
        filters << tr("DeaDBeeF playlist files (*.dbpl)");
        DB_playlist_t **plug = DBAPI->plug_get_playlist_list();
        for (int i = 0; plug[i]; i++) {
            if (plug[i]->extensions && plug[i]->load) {
                const char **exts = plug[i]->extensions;
                if (exts && plug[i]->save)
                    for (int e = 0; exts[e]; e++)
                        filters << QString("*.%1").arg(exts[e]);
            }
        }
    }

    DBAPI->conf_lock();
    QUrl lasturl = QUrl(QString::fromUtf8(DBAPI->conf_get_str_fast(conf_str, "./")));
    DBAPI->conf_unlock();
    setDirectory(lasturl.path());
    setFileMode(mode);
    setOptions(options);
    //setFilter(filters);
    setNameFilters(filters);

    connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

void DBFileDialog::onAccepted() {
    qDebug() << selectedFiles();

    switch (m_type) {
        case OPEN_FILES:
        case ADD_FILES:
            api->addTracks(selectedFiles());
            break;
        case ADD_FOLDERS:
            api->addTracks(selectedFiles(), true);
            break;
        case LOAD_PLAYLIST:
            foreach (QString f, selectedFiles()) {
                api->loadPlaylist(f);
            }
            break;
        case SAVE_PLAYLIST:
            ddb_playlist_t *plt = DBAPI->plt_get_curr();
            if (plt) {
                QString str =  selectedFiles().last();
                int res = DBAPI->plt_save(plt, NULL, NULL,str.toUtf8().constData(), NULL, NULL, NULL);
                if (res) {
                    QMessageBox msg(QMessageBox::Critical,"Save playlist as...", "Saving playlist failed.");
                    msg.exec();
                }
                DBAPI->plt_unref(plt);
            }
            break;


    }

}

/*
QStringList DBFileDialog::exec2() {
    QStringList fileNames;
    if (this->exec())
        fileNames = selectedFiles();
    
    if (fileNames.isEmpty())
        //return 0;
        return fileNames;
    
    if (fileMode() != QFileDialog::Directory && testOption(QFileDialog::ShowDirsOnly)) {
        QStringList path = fileNames.last().split("/");
        path.pop_back();
        QString lastdir = path.join("/");
    }
    DBAPI->conf_set_str("filechooser.lastdir", fileNames.last().toUtf8().constData());
    //return 0;
    return fileNames;
}*/


/*
void DBFileDialog::on_actionAddFiles_triggered() {
    DBFileDialog fileDialog(this,
                            tr("Add file(s) to playlist..."),
                            QStringList(),
                            QFileDialog::ExistingFiles,
                            QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        w->ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void DBFileDialog::on_actionAddFolder_triggered() {
    DBFileDialog fileDialog(this,
                            tr("Add folder(s) to playlist..."),
                            QStringList(),
                            QFileDialog::DirectoryOnly,
                            QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    QStringList fileNames = fileDialog.exec2();
    if (fileNames.isEmpty())
        return;
    foreach (QString localFile, fileNames)
        ui->playList->insertByURLAtPosition(QUrl::fromLocalFile(localFile), DBAPI->pl_getcount(PL_MAIN) - 1);
}

void MainWindow::on_actionAddURL_triggered() {
    ui->playList->insertByURLAtPosition(QUrl::fromUserInput(QInputDialog::getText(this, tr("Enter URL..."), tr("URL: "), QLineEdit::Normal)));
}

void MainWindow::on_actionAddAudioCD_triggered() {
    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, SIGNAL(finished()), ui->playList, SLOT(refresh()));
    watcher->setFuture(QtConcurrent::run(loadAudioCD));
}
*/
