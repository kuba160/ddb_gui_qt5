#ifndef DEADBEEFTRANSLATOR_H
#define DEADBEEFTRANSLATOR_H

#include <QTranslator>

#ifndef DISABLE_GETTEXT
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

#endif // DEADBEEFTRANSLATOR_H
