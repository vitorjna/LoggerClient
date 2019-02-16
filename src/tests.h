#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include <QAbstractSocket>
#include <QApplication>
#include <QDebug>

#include "util/NetworkUtils.h"

class Tests
{
public:
    static void run() {

        qDebug() << NetworkUtils::isIpV4Address("localhost");

        exit(0);
    }
};
