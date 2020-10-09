#include "ChannelSocketClient.h"
#include "network/TcpSocketThreadable.h"
#include "util/MemoryUtils.h"
#include "util/NetworkUtils.h"
#include "util/TimeUtils.h"

ChannelSocketClient::ChannelSocketClient(QObject *parent)
    : QObject(parent)
    , bKeepRetrying(false)
    , nPort(0)
    , mySocket(new TcpSocketThreadable(parent))
{
    setupSignalsAndSlots();

    mySocket->moveToThread(&myWorkerThread);
    QObject::connect(&myWorkerThread, SIGNAL(finished()), this, SLOT(deleteLater()));
    myWorkerThread.start();
}

ChannelSocketClient::~ChannelSocketClient()
{
    bKeepRetrying = false;

    disconnectSocket();

    MemoryUtils::deletePointer(mySocket);

    myWorkerThread.quit();
    myWorkerThread.wait(200);
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

        emit connectSocket(this->szIpAddress, this->nPort);
        return true;
    }

}

void ChannelSocketClient::setNeverDies(bool bNeverDies)
{
    this->bKeepRetrying = bNeverDies;
}

void ChannelSocketClient::disconnectAndStopRetries()
{
    bKeepRetrying = false;
    clearIpAndPort(true);

    emit disconnectSocket();
}

void ChannelSocketClient::clearIpAndPort(bool bForce)
{
    if (bForce == true
        || bKeepRetrying == false) {

        szIpAddress.clear();
        nPort = 0;
    }
}

void ChannelSocketClient::setupSignalsAndSlots()
{
    QObject::connect(mySocket,      SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                     this,          SLOT(socketStateChanged(QAbstractSocket::SocketState)));

    QObject::connect(mySocket,      SIGNAL(error(QAbstractSocket::SocketError)),
                     this,          SLOT(socketError(QAbstractSocket::SocketError)));

    QObject::connect(mySocket,      SIGNAL(newMessage(QString)),
                     this,          SLOT(newMessageOnSocket(QString)));

    QObject::connect(this,          SIGNAL(connectSocket(QString, quint16)),
                     mySocket,      SLOT(connectSocket(QString, quint16)));

    QObject::connect(this,          SIGNAL(disconnectSocket()),
                     mySocket,      SLOT(disconnectSocket()));

}

void ChannelSocketClient::reconnectSocket()
{
    if (bKeepRetrying == true) {
        emit connectSocket(this->szIpAddress, this->nPort);
    }
}

void ChannelSocketClient::newMessageOnSocket(const QString &szMessage)
{
    //check if there was a disconnect requested, ignore message
    if (nPort == 0) {
        return;
    }

    emit newMessageReceived(szMessage);
}

void ChannelSocketClient::socketStateChanged(QAbstractSocket::SocketState eSocketState)
{
    if (nPort == 0) {
        //socket was trying to connect when a disconnect was requested
        if (eSocketState == QAbstractSocket::ConnectedState) {
            emit disconnectSocket();
        }

        return;
    }

#ifdef DEBUG_STUFF
    QMetaEnum myMetaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    qDebug() << "socketStateChanged" << myMetaEnum.valueToKey(eSocketState);
#endif

    switch (eSocketState) {
        case QAbstractSocket::UnconnectedState:
            reconnectSocket();
            break;

        case QAbstractSocket::HostLookupState:
            break;

        case QAbstractSocket::ConnectingState:
            emit connectionInProgress();
            break;

        case QAbstractSocket::ConnectedState:
            emit connectionSuccess();
            break;

        case QAbstractSocket::BoundState:
            break;

        case QAbstractSocket::ListeningState:
            break;

        case QAbstractSocket::ClosingState:
            reconnectSocket();
            break;
    }
}

void ChannelSocketClient::socketError(QAbstractSocket::SocketError eSocketError)
{
    if (nPort == 0) {
        return;
    }

    emit connectionError(mySocket->error(), mySocket->errorString());

    switch (eSocketError) {
        case QAbstractSocket::ConnectionRefusedError:
        case QAbstractSocket::RemoteHostClosedError:
        case QAbstractSocket::HostNotFoundError:
        case QAbstractSocket::SocketAccessError:
        case QAbstractSocket::SocketResourceError:
        case QAbstractSocket::SocketTimeoutError:
        case QAbstractSocket::DatagramTooLargeError:
        case QAbstractSocket::NetworkError:
        case QAbstractSocket::AddressInUseError:
        case QAbstractSocket::SocketAddressNotAvailableError:
        case QAbstractSocket::UnsupportedSocketOperationError:
        case QAbstractSocket::UnfinishedSocketOperationError:
        case QAbstractSocket::ProxyAuthenticationRequiredError:
        case QAbstractSocket::SslHandshakeFailedError:
        case QAbstractSocket::ProxyConnectionRefusedError:
        case QAbstractSocket::ProxyConnectionClosedError:
        case QAbstractSocket::ProxyConnectionTimeoutError:
        case QAbstractSocket::ProxyNotFoundError:
        case QAbstractSocket::ProxyProtocolError:
        case QAbstractSocket::OperationError:
        case QAbstractSocket::SslInternalError:
        case QAbstractSocket::SslInvalidUserDataError:
        case QAbstractSocket::TemporaryError:
        case QAbstractSocket::UnknownSocketError:
            reconnectSocket();
            break;
    }
}
