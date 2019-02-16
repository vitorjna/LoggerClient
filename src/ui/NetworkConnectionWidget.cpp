#include "NetworkConnectionWidget.h"

NetworkConnectionWidget::NetworkConnectionWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();
}

NetworkConnectionWidget::~NetworkConnectionWidget()
{
}

void NetworkConnectionWidget::setMode(const UiMode eUiMode)
{
    switch (eUiMode) {
        case NetworkConnectionWidget::IDLE:
            lineEditServerAddressIpV4->setEnabled(true);
            lineEditServerPort->setEnabled(true);
            buttonConnectToServer->setChecked(false);
            buttonConnectToServer->setText(tr("Connect"));
            break;

        case NetworkConnectionWidget::CONNECTING:
            lineEditServerAddressIpV4->setEnabled(false);
            lineEditServerPort->setEnabled(false);
            buttonConnectToServer->setChecked(true);
            buttonConnectToServer->setText(tr("Cancel"));
            break;

        case NetworkConnectionWidget::CONNECTED:
            lineEditServerAddressIpV4->setEnabled(false);
            lineEditServerPort->setEnabled(false);
            buttonConnectToServer->setChecked(true);
            buttonConnectToServer->setText(tr("Disconnect"));
            break;

        case NetworkConnectionWidget::RETRYING:
            lineEditServerAddressIpV4->setEnabled(false);
            lineEditServerPort->setEnabled(false);
            buttonConnectToServer->setChecked(true);
            buttonConnectToServer->setText(tr("Disconnect"));
            break;

        case COUNT_UI_MODE:
            break;
    }
}

void NetworkConnectionWidget::setIp(const QString &szServerAddressIpV4)
{
    lineEditServerAddressIpV4->setText(szServerAddressIpV4);
}

QString NetworkConnectionWidget::getIp()
{
    return lineEditServerAddressIpV4->text();
}

void NetworkConnectionWidget::setPort(const QString &szServerPort)
{
    lineEditServerPort->setText(szServerPort);
}

QString NetworkConnectionWidget::getPort()
{
    return lineEditServerPort->text();
}

void NetworkConnectionWidget::setupUi()
{
    QHBoxLayout *myServerConnectionLayout = new QHBoxLayout(this);
    myServerConnectionLayout->setContentsMargins(0, 0, 0, 0);
    {
        labelServerAddressIpV4 = new QLabel(this);
        labelServerAddressIpV4->setText(tr("Connection:"));

        lineEditServerAddressIpV4 = new QLineEdit(this);
        lineEditServerAddressIpV4->setPlaceholderText(tr("Server Address"));
//                lineEditServerAddressIpV4->setInputMask("000.000.000.000; ");
        lineEditServerAddressIpV4->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        labelServerAddressIpV4->setBuddy(lineEditServerAddressIpV4);

        QLabel *labelServerAddressSeparator = new QLabel(this);
        labelServerAddressSeparator->setText(QStringLiteral(":"));

        lineEditServerPort = new QLineEdit(this);
        lineEditServerPort->setPlaceholderText(tr("Port"));
//                lineEditServerPort->setInputMask("00000;");
        lineEditServerPort->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

        buttonConnectToServer = new QPushButton(this);
        buttonConnectToServer->setCheckable(true);

        connect(lineEditServerAddressIpV4,  SIGNAL(returnPressed()), buttonConnectToServer, SLOT(click()));
        connect(lineEditServerPort,         SIGNAL(returnPressed()), buttonConnectToServer, SLOT(click()));

        myServerConnectionLayout->addWidget(labelServerAddressIpV4);
        myServerConnectionLayout->addWidget(lineEditServerAddressIpV4);
        myServerConnectionLayout->addWidget(labelServerAddressSeparator);
        myServerConnectionLayout->addWidget(lineEditServerPort);
        myServerConnectionLayout->addSpacing(10);
        myServerConnectionLayout->addWidget(buttonConnectToServer);
    }
}

void NetworkConnectionWidget::setupSignalsAndSlots()
{
    connect(buttonConnectToServer,      SIGNAL(clicked(bool)),
            this,                       SIGNAL(buttonConnectToServerToggled(bool)));

//    connect(lineEditServerAddressIpV4,  SIGNAL(cursorPositionChanged(int, int)),
//            this,                       SLOT(lineEditServerAddressIpV4CursorChanged(int, int)));
}

void NetworkConnectionWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    lineEditServerAddressIpV4->setFocus();
}

