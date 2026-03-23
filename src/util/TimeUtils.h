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

    static void saveTime();

    static void resetSavedTime();

    static int64_t getSavedTimeMilliseconds();
    static int64_t getSavedTimeMicroseconds();

    static QString getCompileDate();

private:
    static QString computeCompileDate();

    static QElapsedTimer *myTimer;

    static int64_t llTimeCount;

};
