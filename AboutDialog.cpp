#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <QFile>
#include <QTextStream>

#include "QtGui.h"

extern DB_gui_t plugin;

AboutDialog::AboutDialog(QWidget *parent) :
        QDialog(parent, Qt::WindowTitleHint),
        ui(new Ui::AboutDialog) {

    ui->setupUi(this);

    char str[64];
    deadbeef->pl_format_title(NULL, -1, str, sizeof(str), -1, "DeaDBeeF-%V");
    setWindowTitle(QString("%1 %2").arg(tr("About")).arg(QString::fromUtf8(str)));

    QFile aboutDBFile(QString::fromUtf8(deadbeef->get_doc_dir()) + "/about.txt");

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
