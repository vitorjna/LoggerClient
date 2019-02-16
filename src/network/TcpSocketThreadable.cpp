#include "TcpSocketThreadable.h"
#include "util/MemoryUtils.h"

TcpSocketThreadable::TcpSocketThreadable(QObject *parent)
    : QTcpSocket() //parent removed, to allow moveToThread
    , myMutex(new QMutex(QMutex::NonRecursive))
    , nTimeout(3000)
{
    Q_UNUSED(parent)

    setSocketOption(QAbstractSocket::LowDelayOption, 1);

    QObject::connect(this,      SIGNAL(readyRead()),
                     this,      SLOT(readNewMessage()));
}

TcpSocketThreadable::~TcpSocketThreadable()
{
    MemoryUtils::deleteMutex(myMutex);
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
            return;
    }

    this->connectToHost(szIpAddress, nPort);
    this->waitForConnected(nTimeout);
}

void TcpSocketThreadable::disconnectSocket()
{
    if (this->state() == QAbstractSocket::ConnectedState) {
        this->disconnectFromHost();
//        mySocket->waitForDisconnected(nTimeout);
    }
}

void TcpSocketThreadable::readNewMessage()
{
    QByteArray caData = this->read(INT16_MAX);

    if (caData.size() == 0) {
        //a reading error occurred. Ignore message
        emit error(QAbstractSocket::UnknownSocketError);

    } else {
#ifdef DEBUG_STUFF
//        QDateTime dt = QDateTime::fromString(szaBytes.left(23), "yyyy-MM-dd HH:mm:ss.zzz");

//        int delay = dt.msecsTo(QDateTime::currentDateTime());
//        qDebug() << "delay" << delay;
#endif

        QString szMessage = QString::fromLatin1(caData);
        emit newMessage(szMessage);
    }
}

