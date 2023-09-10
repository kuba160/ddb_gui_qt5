#include "ScriptableSimple.h"

#include "scriptable/scriptable.h"

#include <QDebug>

ScriptableSimple::ScriptableSimple()
{

}


QMap<QString, ddb_scriptable_item_t *> ScriptableSimple::getSelectors(ddb_scriptable_item_t *s) {
    QMap<QString, ddb_scriptable_item_t *> map;

    int children_count = scriptableItemNumChildren(s);

    qDebug() << "Scriptable simple children count:" << children_count;

    for (int i = 0; i < children_count; i++) {
        ddb_scriptable_item_t *child = scriptableItemChildAtIndex(s,i);
        scriptableKeyValue_t *props = scriptableItemProperties(child);//scriptableItemProperties(child);
        while (props) {
            if (strcmp(props->key, "name") == 0) {
                map.insert(props->value, child);
                break;
            }
            props = props->next;
        }
    }
    return map;
}
