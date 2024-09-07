#include <QObject>
#include <QTest>
#include "Tests.h"
#include "DBApi/All_DBApi.h"

void runTests(DBApi *Api) {
    char *conf_dir = getenv("XDG_CONFIG_HOME");
    if (!conf_dir || !strstr(conf_dir, "/tmp")) {
        qDebug() << "ERROR: Tests require a config in tmp directory. Try running run_deadbeef_tests.sh.";
    }
    else {
        runTests_DBApi(Api);
    }
}

