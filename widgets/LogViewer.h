#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include "DBApi.h"
#include <QWidget>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

class LogViewer : public QWidget, public DBWidget
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr, DBApi *Api =nullptr);
    ~LogViewer();
    static QWidget *constructor(QWidget *parent = nullptr, DBApi *api =nullptr);
    static void callback(struct DB_plugin_s *plugin, uint32_t layers, const char *text, void *ctx);
    QVBoxLayout layout;
    QPlainTextEdit text_block;
    QDialogButtonBox buttons;
    QPushButton *clear_button;
public slots:
    void clear();

};

#endif // LOGVIEWER_H
