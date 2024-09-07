#include "t_ItemImporter.h"
#include <QTest>

#define PM (&api->playlist)

t_ItemImporter::t_ItemImporter(DBApi *Api)
    : QObject{Api}
{
    api = Api;
}

void t_ItemImporter::addFileTest() {
    QFuture ft = PM->runFileImport({"/home/kuba/sources/ddb_gui_qt6/tests/audio/silence.mp3"});
    ft.waitForFinished();
    PlayItemIterator iter = ft.result();
    int i = 0;
    while(iter.getNextIter()) {
        i++;
    }
    QCOMPARE(i, 1);
}

void t_ItemImporter::addFolderTest() {
    QFuture ft = PM->runFolderImport({"/home/kuba/sources/ddb_gui_qt6/tests/audio/"});
    ft.waitForFinished();
    PlayItemIterator iter = ft.result();
    int i = 0;
    while(iter.getNextIter()) {
        i++;
    }
    QCOMPARE(i, 3);
}

void t_ItemImporter::addPlaylistTest() {
    QFuture ft = PM->runPlaylistImport({"/home/kuba/sources/ddb_gui_qt6/tests/audio.m3u"});
    ft.waitForFinished();
    PlayItemIterator iter = ft.result();
    int i = 0;
    while(iter.getNextIter()) {
        i++;
    }
    QCOMPARE(i, 3);
}
