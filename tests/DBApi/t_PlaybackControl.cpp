#include "t_PlaybackControl.h"
#include <QSignalSpy>
#include <QRandomGenerator>

#define PC (&api->playback)

t_PlaybackControl::t_PlaybackControl(DBApi *Api)
    : QObject{Api} {
    api = Api;
}

//void initTestCase();
//void cleanupTestCase();

void t_PlaybackControl::volumeTest_data() {
    QTest::addColumn<double>("volume");
    QTest::addRow("test0,-5.0") << -5.0;
    QTest::addRow("test1,0.0") << 0.0;
    for(int i = 0; i < 10; i++) {
        double test_val = QRandomGenerator::global()->generateDouble() * 50 - 50;
        QTest::addRow("rand%d,%f",i, test_val) << test_val;
    }
}

void t_PlaybackControl::volumeTest() {
    QFETCH(double, volume);
    PlaybackControl *pc = &api->playback;
    float orig = pc->getVolume();

    QSignalSpy spy(pc, &PlaybackControl::volumeChanged);
    if (orig == (float) volume) {
        QSKIP("same volume already applied");
    }
    pc->setVolume(volume);
    QCOMPARE((float) pc->getVolume(), (float) volume);
    QCOMPARE((float) api->deadbeef->volume_get_db(), (float) volume);
    spy.wait();
    QCOMPARE(spy.count(), 1);
    pc->setVolume(orig);
    spy.wait();
}

void t_PlaybackControl::shuffleTest_data() {
    QTest::addColumn<PlaybackControl::shuffle>("shuffle");
    QTest::addRow("PlaybackControl::SHUFFLE_OFF") << PlaybackControl::SHUFFLE_OFF;
    QTest::addRow("PlaybackControl::SHUFFLE_TRACKS") << PlaybackControl::SHUFFLE_TRACKS;
    QTest::addRow("PlaybackControl::SHUFFLE_RANDOM") << PlaybackControl::SHUFFLE_RANDOM;
    QTest::addRow("PlaybackControl::SHUFFLE_ALBUMS") << PlaybackControl::SHUFFLE_ALBUMS;
}

void t_PlaybackControl::shuffleTest() {
    QFETCH(PlaybackControl::shuffle, shuffle);
    PlaybackControl *pc = &api->playback;
    PlaybackControl::shuffle orig = pc->getShuffle();

    QSignalSpy spy(pc, &PlaybackControl::shuffleChanged);
    if (orig == shuffle) {
        QSKIP("same shuffle already applied");
    }
    pc->setShuffle(shuffle);
    QCOMPARE(pc->getShuffle(), shuffle);
    QCOMPARE(api->deadbeef->streamer_get_shuffle(), shuffle);
    spy.wait(1);
    QCOMPARE(spy.count(), 1);
    pc->setShuffle(orig);
    spy.wait(1);
}

void t_PlaybackControl::repeatTest_data() {
    QTest::addColumn<PlaybackControl::repeat>("repeat");
    QTest::addRow("PlaybackControl::REPEAT_ALL") << PlaybackControl::REPEAT_ALL;
    QTest::addRow("PlaybackControl::REPEAT_OFF") << PlaybackControl::REPEAT_OFF;
    QTest::addRow("PlaybackControl::REPEAT_SINGLE") << PlaybackControl::REPEAT_SINGLE;
}

void t_PlaybackControl::repeatTest() {
    QFETCH(PlaybackControl::repeat, repeat);
    PlaybackControl *pc = &api->playback;
    PlaybackControl::repeat orig = pc->getRepeat();

    QSignalSpy spy(pc, &PlaybackControl::repeatChanged);
    if (orig == repeat) {
        QSKIP("same repeat already applied");
    }
    pc->setRepeat(repeat);
    QCOMPARE(pc->getRepeat(), repeat);
    QCOMPARE(api->deadbeef->streamer_get_repeat(), repeat);
    spy.wait(1);
    QCOMPARE(spy.count(), 1);
    pc->setRepeat(orig);
    spy.wait(1);
}

void t_PlaybackControl::stopAfterCurrentTest() {
    bool curr = PC->getStopAfterCurrent();
    QSignalSpy spy(PC, &PlaybackControl::stopAfterCurrentChanged);
    PC->setStopAfterCurrent(!curr);
    QCOMPARE(PC->getStopAfterCurrent(), !curr);
    spy.wait(1);
    QCOMPARE(spy.count(), 1);
    PC->setStopAfterCurrent(curr);
    spy.wait(1);
    QCOMPARE(spy.count(), 2);
}

void t_PlaybackControl::stopAfterAlbumTest() {
    bool curr = PC->getStopAfterAlbum();
    QSignalSpy spy(PC, &PlaybackControl::stopAfterAlbumChanged);
    PC->setStopAfterAlbum(!curr);
    QCOMPARE(PC->getStopAfterAlbum(), !curr);
    spy.wait(1);
    QCOMPARE(spy.count(), 1);
    PC->setStopAfterAlbum(curr);
    QCOMPARE(PC->getStopAfterAlbum(), curr);
    spy.wait(1);
    QCOMPARE(spy.count(), 2);
}

// TODO test state/position / playing/paused/stopped and more
