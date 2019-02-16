#pragma once

#include <QDebug>
#include <QElapsedTimer>

class TimeUtils
{
public:
    static void startTimer();

    static int64_t getTimeMilliseconds(bool bResetTimer = false);

    static int64_t getTimeNanoseconds(bool bResetTimer = false);

    static void printTimeMilliseconds(bool bResetTimer = false);

private:
    static QElapsedTimer *myTimer;

};


