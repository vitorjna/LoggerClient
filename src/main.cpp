#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <QAbstractSocket>
#include <QApplication>
#include <QDebug>

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

    registerMetatypes();

    QApplication loggerClient(argc, argv);
    loggerClient.setStyle("Fusion");

    LoggerClientWidget myLoggerClientWidget;

    QString szFileName;

    for (int nIndex = 1; nIndex < argc; ++nIndex) {
        QString szArg = QString::fromLocal8Bit(argv[nIndex]);

        if (szArg == "-file" && nIndex + 1 < argc) {
            szFileName = QString::fromLocal8Bit(argv[++nIndex]);
        }
    }

    if (szFileName.isEmpty() == false) {
        std::cout << "Opening file: " << szFileName.toLocal8Bit().constData() << std::endl;
        myLoggerClientWidget.openLogFile(szFileName);
    }

    myLoggerClientWidget.show();

    return loggerClient.exec();
}
