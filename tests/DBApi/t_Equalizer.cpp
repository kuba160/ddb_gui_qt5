#include "t_Equalizer.h"
#include <QTest>
#include <QSignalSpy>

#define EQ (&api->eq)

t_Equalizer::t_Equalizer(DBApi *Api)
    : QObject{Api}
{
    api = Api;
}

void t_Equalizer::eqEnableTest() {
    if (!EQ->getEqAvailable()) {
        QSKIP("Equalizer unavailable");
    }

    bool curr = EQ->getEqEnabled();

    QSignalSpy spy(EQ, &Equalizer::eqEnabledChanged);
    EQ->setEqEnabled(!curr);
    spy.wait(1);
    QCOMPARE(EQ->getEqEnabled(), !curr);
    QCOMPARE(spy.count(), 1);
    EQ->setEqEnabled(curr);
    spy.wait(1);
}

void t_Equalizer::eqSetTest() {
    if (!EQ->getEqAvailable()) {
        QSKIP("Equalizer unavailable");
    }

    QVariantList eq_orig = EQ->getEq();

    QVariantList eq_test;
    for(int i = 0 ; i < eq_orig.length(); i++) {
        eq_test.append(i%2 ? 20.0 : -20.0);
    }

    QSignalSpy spy(EQ, &Equalizer::eqChanged);
    EQ->setEq(eq_test);
    spy.wait(1);
    QCOMPARE(EQ->getEq(), eq_test);
    QCOMPARE(spy.count(), 1);
    EQ->setEq(eq_orig);
    spy.wait(1);
}
