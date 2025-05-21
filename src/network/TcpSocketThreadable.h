#pragma once

#include <QtNetwork>

#include "interface/IntMutexable.h"

class TcpSocketThreadable : public QTcpSocket, public IntMutexable
{
    Q_OBJECT

public:
    explicit TcpSocketThreadable(QObject *parent = nullptr);
    ~TcpSocketThreadable() override;

private:
    int nTimeout;

public Q_SLOTS:
    void connectSocket(const QString &szIpAddress, const quint16 nPort);

    void disconnectSocket();

private Q_SLOTS:
    void readNewMessage();

Q_SIGNALS:
    void newMessage(const QString &szMessage);

};


