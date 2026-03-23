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

QString TimeUtils::getCompileDate()
{
    static QString cachedCompileTime = computeCompileDate();
    return cachedCompileTime;
}

QString TimeUtils::computeCompileDate()
{
    char cDate[] = __DATE__; // "Mmm dd yyyy"
    unsigned int compileYear = (cDate[7] - '0') * 1000 + (cDate[8] - '0') * 100 + (cDate[9] - '0') * 10 + (cDate[10] - '0');
    unsigned int compileMonth = (cDate[0] == 'J') ? ((cDate[1] == 'a') ? 1 : ((cDate[2] == 'n') ? 6 : 7))      // Jan, Jun or Jul
                                          : (cDate[0] == 'F') ? 2                                                              // Feb
                                          : (cDate[0] == 'M') ? ((cDate[2] == 'r') ? 3 : 5)                                 // Mar or May
                                          : (cDate[0] == 'A') ? ((cDate[1] == 'p') ? 4 : 8)                                 // Apr or Aug
                                          : (cDate[0] == 'S') ? 9                                                              // Sep
                                          : (cDate[0] == 'O') ? 10                                                             // Oct
                                          : (cDate[0] == 'N') ? 11                                                             // Nov
                                          : (cDate[0] == 'D') ? 12                                                             // Dec
                                                                 : 0;
    unsigned int compileDay = (cDate[4] == ' ') ? (cDate[5] - '0') : (cDate[4] - '0') * 10 + (cDate[5] - '0');

    char IsoDate[] =
        {   static_cast<char>(compileYear/1000 + '0'), static_cast<char>((compileYear % 1000)/100 + '0'), static_cast<char>((compileYear % 100)/10 + '0'), static_cast<char>(compileYear % 10 + '0'),
            '-',  static_cast<char>(compileMonth/10 + '0'), static_cast<char>(compileMonth%10 + '0'),
            '-',  static_cast<char>(compileDay/10 + '0'), static_cast<char>(compileDay%10 + '0'),
            0
        };

    return QString::fromLatin1(IsoDate);
}
