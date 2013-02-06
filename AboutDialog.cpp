#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <QFile>
#include <QTextStream>

#include "QtGui.h"

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

    QString qtguiAbout = QString::fromUtf8("QtGui - user interface for DeaDBeeF player based on Qt library\n\n©") + \
                         QString::fromUtf8("2010 Anton Novikov <tonn.post@gmail.com>\n©") + \
                         QString::fromUtf8("2011 Semen Minyushov <semikmsv@gmail.com>\n©") + \
                         QString::fromUtf8("2013 Karjavin Roman <redpunk231@gmail.com>");

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
