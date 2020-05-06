#include "DBFileDialog.h"

#include <QUrl>

#include "QtGui.h"
#include "MainWindow.h"

DBFileDialog::DBFileDialog(QWidget *parent,
                           const QString &caption,
                           const QStringList &filters,
                           FileMode mode, QFileDialog::Options options): QFileDialog(parent, caption, QString())
{
    char buf[256];
    DBAPI->conf_get_str("filechooser.lastdir", "./", buf, sizeof(buf));
    QUrl lasturl = QUrl(QString::fromUtf8(buf));
    setDirectory(lasturl.path());
    setFileMode(mode);
    setOptions(options);
    //setFilters(filters);
    setNameFilters(filters);
}

QStringList DBFileDialog::exec2() {
    QStringList fileNames;
    if (QFileDialog::exec())
        fileNames = selectedFiles();
    
    if (fileNames.isEmpty())
        //return 0;
        return fileNames;
    
    if (fileMode() != QFileDialog::DirectoryOnly) {
        QStringList path = fileNames.last().split("/");
        path.pop_back();
        QString lastdir = path.join("/");
    }
    DBAPI->conf_set_str("filechooser.lastdir", fileNames.last().toUtf8().constData());
    //return 0;
    return fileNames;
}
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
