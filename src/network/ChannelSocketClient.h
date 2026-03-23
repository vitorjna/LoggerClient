#pragma once

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QWidget>
#include <QtNetwork>

class TcpSocketThreadable;

class ChannelSocketClient : public QObject
{
    Q_OBJECT
    QThread myWorkerThread;

public:
    explicit ChannelSocketClient(QObject *parent = nullptr);
    ~ChannelSocketClient() override;

    bool connect(const QString &szIpAddress, const QString &szPort);

    void setAutoReconnect(bool bAutoReconnect);

    void disconnectAndStopRetries();

    void clearIpAndPort(bool bForce = false);

private:
    void setupSignalsAndSlots();

    bool                    bAutoReconnect;

    QString                 szIpAddress;
    quint16                 nPort;

    TcpSocketThreadable     *mySocket;

private Q_SLOTS:
    void reconnectSocket();
    void newMessageOnSocket(const QString &szMessage);

    void socketStateChanged(QAbstractSocket::SocketState eSocketState);
    void socketError(QAbstractSocket::SocketError eSocketError);

Q_SIGNALS:
    void connectionInProgress();
    void connectionSuccess(const QString &szInfo = QString());
    void connectionError(int nSocketError, const QString &szError);
    void newMessageReceived(const QString &szMessage);

    void connectSocket(const QString &szIpAddress, const quint16 nPort);
    void disconnectSocket();

};


