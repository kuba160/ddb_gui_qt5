#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include "DBApi.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog, public DBWidget {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = 0, DBApi *Api = nullptr);
    virtual ~AboutDialog();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
