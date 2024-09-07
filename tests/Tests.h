#include <dbapi/DBApi.h>

void runTests(DBApi* Api);

#define Q_TEST_RUN(x, api) {auto t = new x(api); QTest::qExec(t); delete t;}
