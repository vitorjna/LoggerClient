#pragma once

#include <QDebug>
#include <QWidget>
#include <QtNetwork>

class NetworkUtils
{
public:
    /**
     * Removed unnecessary zeros from the IP Address, which cause the QTcpSocket to not connect
     * @param szIpAddress
     * @return
     */
    static QString cleanupIpV4Address(const QString &szIpAddress);

    static bool isIpV4Address(const QString &szIpAddress);

    static bool isValidPort(const QString &szPort);
};

