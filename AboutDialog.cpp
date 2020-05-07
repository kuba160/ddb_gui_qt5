#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include "QtGui.h"
#include <deadbeef/deadbeef.h>

#include <QFile>
#include <QTextStream>

AboutDialog::AboutDialog(QWidget *parent) :
        QDialog(parent, Qt::WindowTitleHint),
        ui(new Ui::AboutDialog) {

    ui->setupUi(this);

    char str[64];
    DBAPI->pl_format_title(nullptr, -1, str, sizeof(str), -1, "DeaDBeeF-%V");
    setWindowTitle(QString("%1 %2").arg(tr("About")).arg(QString::fromUtf8(str)));

    QFile aboutDBFile(QString::fromUtf8(DBAPI->get_doc_dir()) + "/about.txt");

    if (aboutDBFile.open(QFile::ReadOnly)) {
        QTextStream s(&aboutDBFile);
        QString about = s.readAll();
        ui->deadbeefAboutText->setText(about);
    }
    else
        ui->deadbeefAboutText->setText(tr("Unable to read file with about information"));

    QString qtguiAbout = QString::fromUtf8(plugin.plugin.copyright);

    ui->qtguiAboutText->setText(qtguiAbout);
}

AboutDialog::~AboutDialog() {
    delete ui;
}

void AboutDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
