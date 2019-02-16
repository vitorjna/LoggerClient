#pragma once

#include <QDebug>
#include <QMutex>

class MemoryUtils
{
public:
    template <typename T>
    inline static void deletePointer(T *&rawPointer) {
        if (rawPointer != nullptr) {
            delete rawPointer;
            rawPointer = nullptr;
        }
    }

    inline static void deleteMutex(QMutex *&myMutex) {
        if (myMutex != nullptr) {
//            myMutex->tryLock(200);
//            myMutex->unlock();

            delete myMutex;
            myMutex = nullptr;
        }
    }

};


