#include "DeadbeefTranslator.h"

DeadbeefTranslator *dbtr = nullptr;

DeadbeefTranslator::DeadbeefTranslator(QObject *parent) : QTranslator(parent) {
}

QString DeadbeefTranslator::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const {
    Q_UNUSED(context);
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);
    return QString(gettext(sourceText));
}
