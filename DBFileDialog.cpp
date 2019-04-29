#include "DBFileDialog.h"

#include <QUrl>

#include <QtGui.h>

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
