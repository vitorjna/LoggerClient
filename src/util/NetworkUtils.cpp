#include "NetworkUtils.h"

bool NetworkUtils::isIpV4Address(const QString &szIpAddress)
{
    if (szIpAddress == QStringLiteral("localhost")) {
        return true;
    }

    QHostAddress myIpAddress(szIpAddress);

    return (myIpAddress.isNull() == false); //if null is false, the IP is valid
}

bool NetworkUtils::isValidPort(const QString &szPort)
{
    bool bPortOk = false;
//    unsigned short nPort = szPort.toUShort(bPortOk);
    szPort.toUShort(&bPortOk);

    return bPortOk; //unsigned short is in the range 0-65535, so the conversion already validates the range
}
