#pragma once

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>

#include "ui/element/Completer.h"
#include "ui/element/LineEditWidget.h"
#include "ui/modal/NetworkAddressesManagerWidget.h"

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

    void setStyleSheet(const QString &szStyleSheet);

    void setMode(const UiMode eUiMode);

    void setName(const QString &szServerName);
    inline QString getName() {
        return lineEditServerName->text();
    }

    void setIp(const QString &szServerIpV4);
    inline QString getIp() {
        return lineEditServerIpV4->text();
    }

    void setPort(const QString &szServerPort);
    inline QString getPort() {
        return lineEditServerPort->text();
    }

    inline QPushButton *getButtonConnectToServer() const { return buttonConnectToServer; }

private:
    void setupUi();
    void setupSignalsAndSlots();

    void completerOptionChosen(const QString &szOption, const NetworkAddressesEnum::Columns eColumn);

    QLabel                          *labelConnection{};
    LineEditWidget                  *lineEditServerName{};
    LineEditWidget                  *lineEditServerIpV4{};
    LineEditWidget                  *lineEditServerPort{};

    QPushButton                     *buttonConnectToServer{};
    QPushButton                     *buttonManageAddresses{};

    Completer                       *completerConnectionName{};
    Completer                       *completerServerIpV4{};
    Completer                       *completerServerPort{};

    NetworkAddressesManagerWidget   *myNetworkAddressesManagerWidget{};

protected Q_SLOTS:
    void lineEditReturnPressed();
    void buttonManageAddressesToggled(bool bButtonState);
    void buttonConnectToServerToggled(bool bButtonState);

    void lineEditServerNameEdited(const QString &szConnectionName);
    void lineEditServerIpV4Edited(const QString &szServerIpV4);
    void lineEditServerPortEdited(const QString &szServerPort);

    void connectionRequested(const QString &szAddressName);

Q_SIGNALS:
    void signalButtonConnectToServerToggled(bool);

    //reimplemented
protected:
    void focusInEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};

