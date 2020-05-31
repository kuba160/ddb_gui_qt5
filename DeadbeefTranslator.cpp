#include "DeadbeefTranslator.h"

DeadbeefTranslator::DeadbeefTranslator(QObject *parent) : QTranslator(parent) {

}

QString DeadbeefTranslator::translate(const char *context, const char *sourceText, const char *disambiguation, int n) const {
    return QString(_(sourceText));
}
