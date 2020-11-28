#include "PluginSettingsWidget.h"

#include <QDebug>

#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>

#include "include/parser.h"
#undef DBAPI
#define DBAPI deadbeef_internal

PluginSettingsWidget::PluginSettingsWidget(ddb_dialog_t *conf, QWidget *parent):
        QGroupBox(parent),
        settingsDialog(conf) {

    setTitle(tr("Settings"));
    configureWidgets();
}

PluginSettingsWidget::~PluginSettingsWidget() {
}

void PluginSettingsWidget::configureWidgets() {
    QFormLayout *layout = new QFormLayout(this);
    char token[MAX_TOKEN];
    const char *script = settingsDialog->layout;
    while ((script = gettoken(script, token))) {
        if (strcmp(token, "property")) {
            qDebug() << "invalid token while loading plugin " << settingsDialog->title << " config dialog: " << token << " at line " << parser_line;
            break;
        }

        char labeltext[MAX_TOKEN];
        script = gettoken_warn_eof(script, labeltext);
        if (!script)
            break;

        char type[MAX_TOKEN];
        script = gettoken_warn_eof(script, type);
        if (!script)
            break;

        if (!strncmp (type, "hbox[", 5) || !strncmp (type, "vbox[", 5)) {
            char semicolon[MAX_TOKEN];
            while ((script = gettoken_warn_eof(script, semicolon)))
                if (!strcmp (semicolon, ";"))
                    break;
            continue;
        }

        if (!script)
            break;

        char key[MAX_TOKEN];
        script = gettoken_warn_eof(script, key);
        if (!script)
            break;

        char def[MAX_TOKEN];
        script = gettoken_warn_eof(script, def);
        if (!script)
            break;

        char value[1000];
        settingsDialog->get_param(key, value, sizeof (value), def);

        if (!strcmp(type, "entry") || !strcmp(type, "password")) {
            label = new QLabel(tr(labeltext), this);
            prop = new QLineEdit(value, this);
            QLineEdit *lineEdit = qobject_cast<QLineEdit *>(prop);
            if (!strcmp(type, "password"))
                lineEdit->setEchoMode(QLineEdit::Password);
            layout->addRow(label, prop);
            connect(lineEdit, SIGNAL(editingFinished()), SLOT(saveProperty()));
        } else if (!strcmp(type, "checkbox")) {
            prop = new QCheckBox(tr(labeltext), this);
            QCheckBox *checkBox = qobject_cast<QCheckBox *>(prop);
            int val = atoi(value);
            checkBox->setChecked(val);
            layout->addRow(prop);
            connect(checkBox, SIGNAL(toggled(bool)), SLOT(saveProperty()));
        } else if (!strcmp(type, "file")) {
            label = new QLabel(tr(labeltext), this);
            prop = new QLineEdit(value, this);
            prop->setEnabled(false);
            //connect((QCheckBox *)prop, SIGNAL(toggled(bool)), ...);
            btn = new QPushButton("…", this);
            //connect(btn, SIGNAL(clicked(bool)), ...);
            hbox = new QHBoxLayout(this);
            hbox->addWidget(prop);
            hbox->addWidget(btn);
            layout->addRow(label, hbox);
        } else if (!strncmp(type, "select[", 7)) {
            int n;
            if (1 != sscanf(type+6, "[%d]", &n))
                break;
            label = new QLabel(tr(labeltext), this);
            prop = new QComboBox(this);
            QComboBox *comboBox = qobject_cast<QComboBox *>(prop);
            for (int i = 0; i < n; i++) {
                char entry[MAX_TOKEN];
                script = gettoken_warn_eof(script, entry);
                if (!script)
                    break;
                comboBox->addItem(entry);
            }
            if (!script)
                break;
            comboBox->setCurrentIndex(atoi(value));
            layout->addRow(prop);
            connect(comboBox, SIGNAL(currentIndexChanged(int)), SLOT(saveProperty()));
        } else if (!strncmp(type, "hscale[", 7) || !strncmp(type, "vscale[", 7) || !strncmp(type, "spinbtn[", 8)) {
            float min, max, step;
            const char *args;
            if (type[0] == 's')
                args = type + 7;
            else
                args = type + 6;
            if (3 != sscanf(args, "[%f,%f,%f]", &min, &max, &step))
                break;
            int invert = 0;
            if (min >= max) {
                float tmp = min;
                min = max;
                max = tmp;
                invert = 1;
            }
            if (step <= 0)
                step = 1;
            if (type[0] == 's') {
                prop = new QSpinBox(this);
                QSpinBox *spinBox = qobject_cast<QSpinBox *>(prop);
                spinBox->setMaximum(max);
                spinBox->setMinimum(min);
                spinBox->setSingleStep(step);
                spinBox->setValue(atof(value));
                connect(spinBox, SIGNAL(editingFinished()), SLOT(saveProperty()));
            } else {
                prop = type[0] == 'h' ? new QSlider(Qt::Horizontal, this) : new QSlider(Qt::Vertical, this);
                QSlider *slider = qobject_cast<QSlider *>(prop);
                slider->setInvertedAppearance(invert);
                slider->setMaximum(max);
                slider->setMinimum(min);
                slider->setSingleStep(step);
                slider->setValue(atof(value));
                connect(slider, SIGNAL(sliderReleased()), SLOT(saveProperty()));
            }
            label = new QLabel(tr(labeltext), this);
            layout->addRow(label, prop);
        }

        if (prop)
            keys.insert(prop, key);

        script = gettoken_warn_eof(script, token);
        if (!script)
            break;

        if (strcmp (token, ";")) {
            qDebug() << "expected `;' while loading plugin " << settingsDialog->title << " config dialog: "<< token << " at line " << parser_line;
            break;
        }
    }
    setLayout(layout);
}

void PluginSettingsWidget::saveProperty() {
    if (QWidget *widget = qobject_cast<QWidget *>(QObject::sender())) {
        QString val = "";
        if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(widget))
            val = lineEdit->text();
        else if (QCheckBox *checkBox = qobject_cast<QCheckBox *>(widget))
            val = checkBox->isChecked() ? "1" : "0";
        else if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget))
            val = QString("%1").arg(comboBox->currentIndex());
        else if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(widget))
            val = QString("%1").arg(spinBox->value());
        else if (QSlider *slider = qobject_cast<QSlider *>(widget))
            val = QString("%1").arg(slider->value());

        DBAPI->conf_set_str(keys.value(widget).toUtf8().constData(), val.toUtf8().constData());
		DBAPI->sendmessage(DB_EV_CONFIGCHANGED, 0, 0, 0);
    }
}
