#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include <QAbstractSocket>
#include <QApplication>
#include <QDebug>

#include "tests.h"
#include "ui/LoggerClientWidget.h"
#include "ui/ToastNotificationWidget.h"

void registerMetatypes();
extern "C" [[noreturn]] void __cxa_pure_virtual(void);

namespace __cxxabiv1
{
extern "C"
void __cxa_pure_virtual(void)
{
    char const msg[] = "pure virtual called\n";
    write(2, msg, sizeof msg - 1);
    std::terminate();
}
} // namespace __cxxabiv1

void registerMetatypes()
{
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    qRegisterMetaType<ToastNotificationWidget::NotificationType>("ToastNotificationWidget::NotificationType");
}


int main(int argc, char **argv)
{
    qsrand(QTime::currentTime().msec());

//    Tests::run();

    registerMetatypes();

    QApplication loggerClient(argc, argv);

    LoggerClientWidget myLoggerClientWidget;
    myLoggerClientWidget.show();

    return loggerClient.exec();
}

