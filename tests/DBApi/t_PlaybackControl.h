#ifndef T_PLAYBACKCONTROL_H
#define T_PLAYBACKCONTROL_H

#include <QObject>
#include <QTest>
#include "dbapi/DBApi.h"

class t_PlaybackControl : public QObject {
    Q_OBJECT
public:
    explicit t_PlaybackControl(DBApi *Api);


private:
    DBApi *api = nullptr;

    bool myCondition()
    {
        return true;
    }

private slots:
    // void initTestCase();
    // void cleanupTestCase();

    void volumeTest_data();
    void volumeTest();

    void shuffleTest_data();
    void shuffleTest();

    void repeatTest_data();
    void repeatTest();

    void stopAfterCurrentTest();
    void stopAfterAlbumTest();

};

#endif // T_PLAYBACKCONTROL_H
