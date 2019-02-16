#pragma once

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

class NetworkConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    enum UiMode {
        IDLE,
        CONNECTING,
        CONNECTED,
        RETRYING,
        COUNT_UI_MODE
    };

    explicit NetworkConnectionWidget(QWidget *parent = nullptr);
    ~NetworkConnectionWidget() override;

    void setMode(const UiMode eUiMode);

    void setIp(const QString &szServerAddressIpV4);
    QString getIp();

    void setPort(const QString &szServerPort);
    QString getPort();

    inline QLineEdit    *getLineEditServerAddressIpV4()     const { return lineEditServerAddressIpV4; }
    inline QLineEdit    *getLineEditServerPort()            const { return lineEditServerPort; }
    inline QPushButton  *getButtonConnectToServer()         const { return buttonConnectToServer; }

private:
    void setupUi();
    void setupSignalsAndSlots();

    QLabel                       *labelServerAddressIpV4{};
    QLineEdit                    *lineEditServerAddressIpV4{};
    QLineEdit                    *lineEditServerPort{};
    QPushButton                  *buttonConnectToServer{};

signals:
    void buttonConnectToServerToggled(bool);

//reimplemented
protected:
    void focusInEvent(QFocusEvent *event) override;

};

