#pragma once

#include <QDebug>
#include <QWidget>
#include <QtNetwork>

class NetworkUtils
{
public:
    static bool isIpV4Address(const QString &szIpAddress);

    static bool isValidPort(const QString &szPort);
};

