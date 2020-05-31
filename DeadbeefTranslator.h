#ifndef DEADBEEFTRANSLATOR_H
#define DEADBEEFTRANSLATOR_H

#include <QTranslator>

//#define DISABLE_GETTEXT

#ifdef USE_GETTEXT
# include <libintl.h>
#define _(s) gettext(s)
#else
#define _(s) (s)
#endif

class DeadbeefTranslator : public QTranslator
{
public:
    DeadbeefTranslator(QObject *parent = nullptr);
    QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const;
};

extern DeadbeefTranslator *dbtr;

#endif // DEADBEEFTRANSLATOR_H
