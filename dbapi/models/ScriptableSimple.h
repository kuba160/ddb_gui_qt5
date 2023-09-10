#ifndef SCRIPTABLESIMPLE_H
#define SCRIPTABLESIMPLE_H

#include <QMap>
#include <deadbeef/deadbeef.h>

class ScriptableSimple
{
public:
    ScriptableSimple();

    static QMap<QString, ddb_scriptable_item_t *> getSelectors(ddb_scriptable_item_t *s);
};

#endif // SCRIPTABLESIMPLE_H
