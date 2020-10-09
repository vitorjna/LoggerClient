#include "NetworkUtils.h"

QString NetworkUtils::cleanupIpV4Address(const QString &szIpAddress)
{
    QString szIpAddressClean;
    szIpAddressClean.reserve(szIpAddress.size());

    bool bCanIgnore = false;

    for (int nIndex = 0; nIndex < szIpAddress.size(); ++nIndex) {
        const QChar cChar = szIpAddress[nIndex];

        if (cChar == '.') {
            bCanIgnore = true;

            szIpAddressClean.push_back(cChar);

        } else if (bCanIgnore == true
                   && cChar == '0') {
            continue;

        } else {
            bCanIgnore = false;
            szIpAddressClean.push_back(cChar);
        }

    }

    if (NetworkUtils::isIpV4Address(szIpAddressClean) == true) {
        return szIpAddressClean;

    } else {
        return szIpAddress;
    }
}

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
