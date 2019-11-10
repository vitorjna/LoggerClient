#include "TimeUtils.h"

QElapsedTimer *TimeUtils::myTimer = new QElapsedTimer();

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

