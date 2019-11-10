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

public slots:
    void connectSocket(const QString &szIpAddress, const quint16 nPort);

    void disconnectSocket();

private slots:
    void readNewMessage();

signals:
    void newMessage(const QString &szMessage);

};


