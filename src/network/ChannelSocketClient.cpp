#include "ChannelSocketClient.h"
#include "network/TcpSocketThreadable.h"
#include "util/MemoryUtils.h"
#include "util/NetworkUtils.h"

ChannelSocketClient::ChannelSocketClient(QObject *parent)
    : QObject(parent)
    , bAutoReconnect(false)
    , nPort(0)
    , mySocket(new TcpSocketThreadable(nullptr))
{
    setupSignalsAndSlots();

    mySocket->moveToThread(&myWorkerThread);
    QObject::connect(&myWorkerThread, &QThread::finished, this, &QObject::deleteLater);
    myWorkerThread.start();
}

ChannelSocketClient::~ChannelSocketClient()
{
    bAutoReconnect = false;

    Q_EMIT disconnectSocket();

    if (mySocket) {
        mySocket->deleteLater();
        mySocket = nullptr;
    }

    myWorkerThread.quit();
    if (!myWorkerThread.wait(1000)) {
        myWorkerThread.terminate();
        myWorkerThread.wait();
    }
}

bool ChannelSocketClient::connect(const QString &szIpAddress, const QString &szPort)
{
    qDebug() << "Connecting to" << szIpAddress << ":" << szPort;

    if (NetworkUtils::isIpV4Address(szIpAddress) == false
        || NetworkUtils::isValidPort(szPort) == false) {

        return false;

    } else {
        this->szIpAddress = NetworkUtils::cleanupIpV4Address(szIpAddress);
        this->nPort = szPort.toUShort();

        Q_EMIT connectSocket(this->szIpAddress, this->nPort);
        return true;
    }

}

void ChannelSocketClient::setAutoReconnect(bool bAutoReconnect)
{
    this->bAutoReconnect = bAutoReconnect;
}

void ChannelSocketClient::disconnectAndStopRetries()
{
    bAutoReconnect = false;
    clearIpAndPort(true);

    Q_EMIT disconnectSocket();
}

void ChannelSocketClient::clearIpAndPort(bool bForce)
{
    if (bForce == true
        || bAutoReconnect == false) {

        szIpAddress.clear();
        nPort = 0;
    }
}

void ChannelSocketClient::setupSignalsAndSlots()
{
    QObject::connect(mySocket,      &QAbstractSocket::stateChanged,
                     this,          &ChannelSocketClient::socketStateChanged);

    QObject::connect(mySocket,      &QAbstractSocket::errorOccurred,
                     this,          &ChannelSocketClient::socketError);

    QObject::connect(mySocket,      &TcpSocketThreadable::newMessage,
                     this,          &ChannelSocketClient::newMessageOnSocket);

    QObject::connect(this,          &ChannelSocketClient::connectSocket,
                     mySocket,      &TcpSocketThreadable::connectSocket);

    QObject::connect(this,          &ChannelSocketClient::disconnectSocket,
                     mySocket,      &TcpSocketThreadable::disconnectSocket);

}

void ChannelSocketClient::reconnectSocket()
{
    if (bAutoReconnect == true) {
        Q_EMIT connectSocket(this->szIpAddress, this->nPort);
    }
}

void ChannelSocketClient::newMessageOnSocket(const QString &szMessage)
{
    //check if there was a disconnect requested, ignore message
    if (nPort == 0) {
        return;
    }

    Q_EMIT newMessageReceived(szMessage);
}

void ChannelSocketClient::socketStateChanged(QAbstractSocket::SocketState eSocketState)
{
    if (nPort == 0) {
        //socket was trying to connect when a disconnect was requested
        if (eSocketState == QAbstractSocket::ConnectedState) {
            Q_EMIT disconnectSocket();
        }

        return;
    }

#ifdef DEBUG_STUFF
    const QMetaEnum myMetaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    qDebug() << "socketStateChanged" << myMetaEnum.valueToKey(eSocketState);
#endif

    switch (eSocketState) {
        case QAbstractSocket::UnconnectedState:
            reconnectSocket();
            break;

        case QAbstractSocket::HostLookupState:
            break;

        case QAbstractSocket::ConnectingState:
            Q_EMIT connectionInProgress();
            break;

        case QAbstractSocket::ConnectedState:
            Q_EMIT connectionSuccess();
            break;

        case QAbstractSocket::BoundState:
            break;

        case QAbstractSocket::ListeningState:
            break;

        case QAbstractSocket::ClosingState:
            break;
    }
}

void ChannelSocketClient::socketError(QAbstractSocket::SocketError eSocketError)
{
    if (nPort == 0) {
        return;
    }

    // Use the error information passed by the signal if possible,
    // or rely on the fact that QAbstractSocket signals often provide enough context.
    // However, TcpSocketThreadable is a QTcpSocket, so we can't easily get errorString
    // without cross-thread access if we didn't pass it in the signal.
    // In Qt, socketError signal actually passes the error.
    // To be safe and avoid cross-thread access to mySocket from the UI thread:
    Q_EMIT connectionError(static_cast<int>(eSocketError), tr("Socket error occurred"));

    // Reconnection is handled by UnconnectedState in socketStateChanged
}
