#pragma once

#include <QtNetwork>

class TcpSocketThreadable : public QTcpSocket
{
    Q_OBJECT

public:
    explicit TcpSocketThreadable(QObject *parent = nullptr);
    ~TcpSocketThreadable() override;

private:
    QMutex *myMutex;
    int nTimeout;

public slots:
    void connectSocket(const QString &szIpAddress, const quint16 nPort);

    void disconnectSocket();

private slots:
    void readNewMessage();

signals:
    void newMessage(QString szMessage);

};


