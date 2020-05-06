#ifndef DBFILEDIALOG_H
#define DBFILEDIALOG_H

#include <QFileDialog>


class DBFileDialog : public QFileDialog {
    Q_OBJECT
public:
    DBFileDialog(QWidget *parent,
                 const QString &caption = QString(),
                 const QStringList &filters = QStringList(),
                 FileMode mode = QFileDialog::AnyFile,
                 QFileDialog::Options options = QFileDialog::DontUseNativeDialog);

    QStringList exec2();
    //virtual int exec();

public slots:
    //void on_actionAddFiles_triggered();
    //void on_actionAddFolder_triggered();
};

#endif // DBFILEDIALOG_H
