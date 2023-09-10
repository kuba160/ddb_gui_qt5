#include "ActionsDefault.h"




void insertDefaultActions(DBApi *api) {
    //api.actions->i;
    ActionsConfig *a_c = new ActionsConfig(api, api);
    ActionsDeadbeef *a_d = new ActionsDeadbeef(api, api);
    ActionsPlaceholder *a_p = new ActionsPlaceholder(api);

    api->actions.registerActionOwner(a_c);
    api->actions.registerActionOwner(a_d);
    api->actions.registerActionOwner(a_p);
}

void freeDefaultActions(DBApi *api) {

}
