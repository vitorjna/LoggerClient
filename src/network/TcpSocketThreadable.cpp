#include "TcpSocketThreadable.h"
#include "util/MemoryUtils.h"

TcpSocketThreadable::TcpSocketThreadable(QObject *parent)
    : QTcpSocket(nullptr) //parent removed, to allow moveToThread
    , IntMutexable()
    , nTimeout(3000)
{
    Q_UNUSED(parent)

    setSocketOption(QAbstractSocket::LowDelayOption, 1);
    setProxy(QNetworkProxy::NoProxy); //TODO make proxy usage as an option

    QObject::connect(this, &TcpSocketThreadable::readyRead,
                     this, &TcpSocketThreadable::readNewMessage);
}

TcpSocketThreadable::~TcpSocketThreadable()
{
}

void TcpSocketThreadable::connectSocket(const QString &szIpAddress, const quint16 nPort)
{
    if (szIpAddress.size() == 0) {
        return;
    }

    QMutexLocker myScopedMutex(myMutex);

    switch (this->state()) {
        case QAbstractSocket::HostLookupState:
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::ConnectedState:
        case QAbstractSocket::ListeningState:
        case QAbstractSocket::BoundState:
            return; //already connected, or trying to connect

        case QAbstractSocket::UnconnectedState:
        case QAbstractSocket::ClosingState:
            break;
    }

    connectToHost(szIpAddress, nPort);
    waitForConnected(nTimeout);
}

void TcpSocketThreadable::disconnectSocket()
{
    if (this->state() == QAbstractSocket::ConnectedState) {
        disconnectFromHost();
//        mySocket->waitForDisconnected(nTimeout);
    }
}

void TcpSocketThreadable::readNewMessage()
{
    const QByteArray caData = this->read(INT16_MAX);

    if (caData.size() == 0) {
        //a reading error occurred. Ignore message
        emit errorOccurred(QAbstractSocket::UnknownSocketError);

    } else {
#ifdef DEBUG_STUFF
//        QDateTime dt = QDateTime::fromString(szaBytes.left(23), "yyyy-MM-dd HH:mm:ss.zzz");

//        int delay = dt.msecsTo(QDateTime::currentDateTime());
//        qDebug() << "delay" << delay;
#endif

        const QString szMessage = QString::fromLatin1(caData);
        emit newMessage(szMessage);
    }
}

