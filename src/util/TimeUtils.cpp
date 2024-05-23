#include "TimeUtils.h"

QElapsedTimer *TimeUtils::myTimer = new QElapsedTimer();
int64_t TimeUtils::llTimeCount = 0;

void TimeUtils::startTimer()
{
    myTimer->start();
}

int64_t TimeUtils::getTimeMilliseconds(bool bResetTimer)
{
    const int64_t llTimeElapsed = myTimer->elapsed();

    if (bResetTimer == true) {
        myTimer->restart();
    }

    return llTimeElapsed;
}

int64_t TimeUtils::getTimeNanoseconds(bool bResetTimer)
{
    const int64_t llTimeElapsed = myTimer->nsecsElapsed();

    if (bResetTimer == true) {
        myTimer->restart();
    }

    return llTimeElapsed;
}

void TimeUtils::printTimeMilliseconds(bool bResetTimer)
{
    qDebug() << getTimeMilliseconds(bResetTimer);
}

void TimeUtils::saveTime()
{
    llTimeCount += myTimer->nsecsElapsed();
}

void TimeUtils::resetSavedTime()
{
    llTimeCount = 0;
}

int64_t TimeUtils::getSavedTimeMilliseconds()
{
    return llTimeCount / 1000000;
}

int64_t TimeUtils::getSavedTimeMicroseconds()
{
    return llTimeCount / 1000;
}
