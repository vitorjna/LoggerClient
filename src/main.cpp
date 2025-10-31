#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <QAbstractSocket>
#include <QApplication>
#include <QDebug>

#include "tests.h"
#include "application/GlobalConstants.h"
#include "ui/LoggerClientWidget.h"
#include "ui/ToastNotificationWidget.h"

void registerMetatypes();

void registerMetatypes()
{
    qRegisterMetaType<GlobalConstants::ErrorCode>("GlobalConstants::ErrorCode");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");
    qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    qRegisterMetaType<ToastNotificationWidget::NotificationType>("ToastNotificationWidget::NotificationType");
}


int main(int argc, char **argv)
{
    srand(QTime::currentTime().msec());

    // QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

//    Tests::run();

    registerMetatypes();

    QApplication loggerClient(argc, argv);
    loggerClient.setStyle("Fusion");

    LoggerClientWidget myLoggerClientWidget;
    myLoggerClientWidget.show();

    return loggerClient.exec();
}

