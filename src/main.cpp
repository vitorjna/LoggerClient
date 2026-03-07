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

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

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

    QString szFileName;
    QString szIp;
    QString szPort;
    QString szPattern;
    bool bHelp = false;

    for (int nIndex = 1; nIndex < argc; ++nIndex) {
        QString szArg = QString::fromLocal8Bit(argv[nIndex]);

        if (szArg == "-help" || szArg == "--help") {
            bHelp = true;
            break;

        } else if (szArg == "-file" && nIndex + 1 < argc) {
            szFileName = QString::fromLocal8Bit(argv[++nIndex]);

        } else if (szArg == "-ip" && nIndex + 1 < argc) {
            szIp = QString::fromLocal8Bit(argv[++nIndex]);

        } else if (szArg == "-port" && nIndex + 1 < argc) {
            szPort = QString::fromLocal8Bit(argv[++nIndex]);

        } else if (szArg == "-pattern" && nIndex + 1 < argc) {
            szPattern = QString::fromLocal8Bit(argv[++nIndex]);
        }
    }

    if (bHelp == true) {
#ifdef Q_OS_WIN
        AttachConsole(ATTACH_PARENT_PROCESS);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
#endif

        std::cout << "Usage: LoggerClient [OPTIONS]" << std::endl;
        std::cout << "  -file <filename>  Open a log file" << std::endl;
        std::cout << "  -ip <ip>          IP to connect to" << std::endl;
        std::cout << "  -port <port>      Port to connect to" << std::endl;
        std::cout << "  -pattern <regex>  Pattern to use" << std::endl;
        std::cout << "  -help             Show this help message" << std::endl;

        return 0;
    }

    QApplication loggerClient(argc, argv);
    loggerClient.setStyle("Fusion");

    LoggerClientWidget myLoggerClientWidget;

    if (szFileName.isEmpty() == false) {
        std::cout << "Opening file: " << szFileName.toLocal8Bit().constData() << std::endl;
        myLoggerClientWidget.openLogFile(szFileName);
        // if file will be open, IP and Port can be ignored
        szIp.clear();
        szPort.clear();
    }

    if (szIp.isEmpty() == false || szPort.isEmpty() == false || szPattern.isEmpty() == false) {
        myLoggerClientWidget.setupCommandLineArgs(szIp, szPort, szPattern);
    }

    myLoggerClientWidget.show();

    return loggerClient.exec();
}
