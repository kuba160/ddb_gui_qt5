#include "All_DBApi.h"
#include "../Tests.h"

#include "t_PlaybackControl.h"
#include "t_PlaylistManager.h"
#include "t_ItemImporter.h"
#include "t_Equalizer.h"

void runTests_DBApi(DBApi *Api) {
    Q_TEST_RUN(t_PlaybackControl, Api);
    Q_TEST_RUN(t_ItemImporter, Api);
    Q_TEST_RUN(t_PlaylistManager, Api);
    Q_TEST_RUN(t_Equalizer, Api);
}
