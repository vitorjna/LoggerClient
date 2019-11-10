#pragma once

#include <QDebug>
#include <QMutex>

#include "util/MemoryUtils.h"

/**
 * Allows a class to have a mutex object, while handling the life of the mutex
 * Note: not the clean way, but very practical. May remove this if the negatives outweigh the positives
 */
class IntMutexable
{
public:
    explicit IntMutexable(const QMutex::RecursionMode eRecursionMode = QMutex::NonRecursive)
        : myMutex(new QMutex(eRecursionMode)) {
    }

    ~IntMutexable() {
        MemoryUtils::deleteMutex(myMutex);
    }

    /// usage: QMutexLocker myScopedMutex(myMutex);
    QMutex *myMutex;

};

