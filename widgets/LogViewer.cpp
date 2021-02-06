#include "LogViewer.h"

LogViewer::LogViewer(QWidget *parent, DBApi *Api) : QWidget(parent),
                                                    DBWidget(parent,Api),
                                                    text_block(this),
                                                    buttons(this) {
    setLayout(&layout);
    layout.addWidget(&text_block);
    layout.addWidget(&buttons,0,Qt::AlignRight);
    DBAPI->log_viewer_register(callback,this);

    clear_button = buttons.addButton(tr("Clear"),QDialogButtonBox::YesRole);
    connect(&buttons, SIGNAL(accepted()), this, SLOT(clear()));

    text_block.setReadOnly(true);
}

LogViewer::~LogViewer() {
    DBAPI->log_viewer_unregister(callback,this);
}

QWidget *LogViewer::constructor(QWidget *parent, DBApi *api) {
    return new LogViewer(parent,api);
}

void LogViewer::clear() {
    text_block.clear();
}

void LogViewer::callback(struct DB_plugin_s *plugin, uint32_t layers, const char *text, void *ctx) {
    // TODO filter
    LogViewer *log = static_cast<LogViewer *>(ctx);
    QString t = text;
    t.remove(t.length()-1,1);
    log->text_block.appendPlainText(t);

}
