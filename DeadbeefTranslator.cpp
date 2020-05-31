#include "DeadbeefTranslator.h"

DeadbeefTranslator::DeadbeefTranslator(QObject *parent) : QTranslator(parent) {

}


QString translate(const char *context, const char *sourceText, const char *disambiguation, int n) {
    return QString(_(sourceText));
}
