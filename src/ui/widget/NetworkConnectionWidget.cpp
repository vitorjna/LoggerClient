#include "NetworkConnectionWidget.h"
#include "ui/ToastNotificationWidget.h"

NetworkConnectionWidget::NetworkConnectionWidget(QWidget *parent)
    : QWidget(parent)
    , myNetworkAddressesManagerWidget(new NetworkAddressesManagerWidget(this))
{
    setupUi();
    setupSignalsAndSlots();

    setMode(IDLE);
}

NetworkConnectionWidget::~NetworkConnectionWidget() = default;

void NetworkConnectionWidget::setStyleSheet(const QString &szStyleSheet)
{
    if (szStyleSheet.isEmpty() == false) {
        QWidget::setStyleSheet(szStyleSheet);
    }
}

void NetworkConnectionWidget::setMode(const UiMode eUiMode)
{
    switch (eUiMode) {
        case NetworkConnectionWidget::IDLE:
            lineEditServerName->setEnabled(true);
            lineEditServerIpV4->setEnabled(true);
            lineEditServerPort->setEnabled(true);
            buttonConnectToServer->setChecked(false);
            buttonConnectToServer->setText(tr("&Connect"));
            break;

        case NetworkConnectionWidget::CONNECTING:
            lineEditServerName->setEnabled(false);
            lineEditServerIpV4->setEnabled(false);
            lineEditServerPort->setEnabled(false);
            buttonConnectToServer->setChecked(true);
            buttonConnectToServer->setText(tr("&Cancel"));
            break;

        case NetworkConnectionWidget::CONNECTED:
            lineEditServerName->setEnabled(false);
            lineEditServerIpV4->setEnabled(false);
            lineEditServerPort->setEnabled(false);
            buttonConnectToServer->setChecked(true);
            buttonConnectToServer->setText(tr("&Disconnect"));
            break;

        case NetworkConnectionWidget::RETRYING:
            lineEditServerName->setEnabled(false);
            lineEditServerIpV4->setEnabled(false);
            lineEditServerPort->setEnabled(false);
            buttonConnectToServer->setChecked(true);
            buttonConnectToServer->setText(tr("&Disconnect"));
            break;

        case COUNT_UI_MODE:
            break;
    }
}

void NetworkConnectionWidget::setName(const QString &szServerName)
{
    lineEditServerName->setText(szServerName);
}

void NetworkConnectionWidget::setIp(const QString &szServerIpV4)
{
    lineEditServerIpV4->setText(szServerIpV4);
}

void NetworkConnectionWidget::setPort(const QString &szServerPort)
{
    lineEditServerPort->setText(szServerPort);
}

void NetworkConnectionWidget::setupUi()
{
    QHBoxLayout *myServerConnectionLayout = new QHBoxLayout(this);
    myServerConnectionLayout->setContentsMargins(0, 0, 0, 0);
    myServerConnectionLayout->setSpacing(0);
    {
        labelConnection = new QLabel(this);
        labelConnection->setText(tr("Connection:"));

        QSizePolicy mySizePolicyLarge;
        mySizePolicyLarge.setHorizontalStretch(2);
        mySizePolicyLarge.setHorizontalPolicy(QSizePolicy::Minimum);
        mySizePolicyLarge.setVerticalPolicy(QSizePolicy::Fixed);

        QSizePolicy mySizePolicySmall;
        mySizePolicySmall.setHorizontalStretch(1);
        mySizePolicySmall.setHorizontalPolicy(QSizePolicy::Minimum);
        mySizePolicySmall.setVerticalPolicy(QSizePolicy::Fixed);

        lineEditServerName = new LineEditWidget(this);
        lineEditServerName->setPlaceholderText(tr("Server Name"));
        lineEditServerName->setToolTip(tr("Server Name"));
//        lineEditServerName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEditServerName->setSizePolicy(mySizePolicyLarge);
        lineEditServerName->setClearButtonEnabled(true);
        labelConnection->setBuddy(lineEditServerName);

        QLabel *labelNameOrIp = new QLabel(this);
        labelNameOrIp->setText(tr("or"));


        lineEditServerIpV4 = new LineEditWidget(this);
        lineEditServerIpV4->setPlaceholderText(tr("Server IP"));
        lineEditServerIpV4->setToolTip(tr("Server IP"));
        //                lineEditServerIpV4->setInputMask("000.000.000.000; ");
//        lineEditServerIpV4->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEditServerIpV4->setSizePolicy(mySizePolicyLarge);

        QLabel *labelServerAddressSeparator = new QLabel(this);
        labelServerAddressSeparator->setText(QStringLiteral(":"));

        lineEditServerPort = new LineEditWidget(this);
        lineEditServerPort->setPlaceholderText(tr("Port"));
        lineEditServerPort->setToolTip(tr("Port"));
//                lineEditServerPort->setInputMask("00000;");
//        lineEditServerPort->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        lineEditServerPort->setSizePolicy(mySizePolicySmall);


        buttonConnectToServer = new QPushButton(this);
        buttonConnectToServer->setCheckable(true);

        buttonManageAddresses = new QPushButton(this);
        buttonManageAddresses->setCheckable(true);
        buttonManageAddresses->setText(tr("Manage"));
        buttonManageAddresses->setToolTip(tr("Manage stored addresses"));


        myServerConnectionLayout->addWidget(labelConnection);
        myServerConnectionLayout->addWidget(lineEditServerName);
        myServerConnectionLayout->addWidget(labelNameOrIp);
        myServerConnectionLayout->addWidget(lineEditServerIpV4);
        myServerConnectionLayout->addWidget(labelServerAddressSeparator);
        myServerConnectionLayout->addWidget(lineEditServerPort);
        myServerConnectionLayout->addSpacing(10);
        myServerConnectionLayout->addWidget(buttonConnectToServer);
        myServerConnectionLayout->addWidget(buttonManageAddresses);
    }

    completerConnectionName = new Completer(static_cast<QAbstractItemModel *>(myNetworkAddressesManagerWidget->getModel()), this);
    completerConnectionName->setCompletionColumn(NetworkAddressesEnum::COLUMN_ADDRESS_NAME);
    completerConnectionName->setCompletionMode(QCompleter::PopupCompletion);
    completerConnectionName->setCaseSensitivity(Qt::CaseInsensitive);
    completerConnectionName->setFilterMode(Qt::MatchContains);
    lineEditServerName->setCompleter(completerConnectionName);

    completerServerIpV4 = new Completer(static_cast<QAbstractItemModel *>(myNetworkAddressesManagerWidget->getModel()), this);
    completerServerIpV4->setCompletionColumn(NetworkAddressesEnum::COLUMN_SERVER_IP);
    completerServerIpV4->setCompletionMode(QCompleter::PopupCompletion);
    completerServerIpV4->setCaseSensitivity(Qt::CaseInsensitive);
    completerServerIpV4->setFilterMode(Qt::MatchContains);
    lineEditServerIpV4->setCompleter(completerServerIpV4);

    completerServerPort = new Completer(static_cast<QAbstractItemModel *>(myNetworkAddressesManagerWidget->getModel()), this);
    completerServerPort->setCompletionColumn(NetworkAddressesEnum::COLUMN_SERVER_PORT);
    completerServerPort->setCompletionMode(QCompleter::PopupCompletion);
    completerServerPort->setCaseSensitivity(Qt::CaseInsensitive);
    completerServerPort->setFilterMode(Qt::MatchContains);
    lineEditServerPort->setCompleter(completerServerPort);
}

void NetworkConnectionWidget::setupSignalsAndSlots()
{
    connect(lineEditServerName,                 &QLineEdit::returnPressed,
            this,                               &NetworkConnectionWidget::lineEditReturnPressed);
    connect(lineEditServerIpV4,                 &QLineEdit::returnPressed,
            this,                               &NetworkConnectionWidget::lineEditReturnPressed);
    connect(lineEditServerPort,                 &QLineEdit::returnPressed,
            this,                               &NetworkConnectionWidget::lineEditReturnPressed);

    connect(buttonConnectToServer,              &QPushButton::toggled,
            this,                               &NetworkConnectionWidget::buttonConnectToServerToggled);

    connect(buttonManageAddresses,              &QPushButton::toggled,
            this,                               &NetworkConnectionWidget::buttonManageAddressesToggled);

    connect(myNetworkAddressesManagerWidget,    &NetworkAddressesManagerWidget::aboutToHide,
            this,                               [ = ] { buttonManageAddresses->setChecked(false); } );

    connect(myNetworkAddressesManagerWidget,    &NetworkAddressesManagerWidget::connectionRequested,
            this,                               &NetworkConnectionWidget::connectionRequested);

    connect(lineEditServerName,                 &LineEditWidget::textEdited,
            this,                               &NetworkConnectionWidget::lineEditServerNameEdited);

    connect(lineEditServerIpV4,                 &LineEditWidget::textEdited,
            this,                               &NetworkConnectionWidget::lineEditServerIpV4Edited);

    connect(lineEditServerPort,                 &LineEditWidget::textEdited,
            this,                               &NetworkConnectionWidget::lineEditServerPortEdited);

    connect(completerConnectionName,            QOverload<const QString &>::of(&QCompleter::activated),
    this,                               [ = ] (const QString & text) { completerOptionChosen(text, NetworkAddressesEnum::COLUMN_ADDRESS_NAME); } );

    connect(completerServerIpV4,                QOverload<const QString &>::of(&QCompleter::activated),
    this,                               [ = ] (const QString & text) { completerOptionChosen(text, NetworkAddressesEnum::COLUMN_SERVER_IP); } );

    connect(completerServerPort,                QOverload<const QString &>::of(&QCompleter::activated),
    this,                               [ = ] (const QString & text) { completerOptionChosen(text, NetworkAddressesEnum::COLUMN_SERVER_PORT); } );
}

void NetworkConnectionWidget::completerOptionChosen(const QString &szOption, const NetworkAddressesEnum::Columns eColumn)
{
    QStringList szaRow = myNetworkAddressesManagerWidget->getRow(szOption, eColumn);

    if (szaRow.size() != NetworkAddressesEnum::COUNT_TABLE_COLUMNS) {
        ToastNotificationWidget::showMessage(this, tr("Search failed"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    lineEditServerName->setText(szaRow.at(NetworkAddressesEnum::COLUMN_ADDRESS_NAME));
    lineEditServerIpV4->setText(szaRow.at(NetworkAddressesEnum::COLUMN_SERVER_IP));
    lineEditServerPort->setText(szaRow.at(NetworkAddressesEnum::COLUMN_SERVER_PORT));

    lineEditReturnPressed();
}

void NetworkConnectionWidget::lineEditReturnPressed()
{
    if (lineEditServerIpV4->text().isEmpty() == false
        && lineEditServerPort->text().isEmpty() == false) {

        buttonConnectToServer->click();
    }
}

void NetworkConnectionWidget::buttonManageAddressesToggled(bool bButtonState)
{
    if (bButtonState == true) {
        myNetworkAddressesManagerWidget->setValues(lineEditServerName->text(), lineEditServerIpV4->text(), lineEditServerPort->text());

        myNetworkAddressesManagerWidget->show();

    } else {
        myNetworkAddressesManagerWidget->hide();
        buttonManageAddresses->clearFocus();
    }
}

void NetworkConnectionWidget::buttonConnectToServerToggled(bool bButtonState)
{
    emit signalButtonConnectToServerToggled(bButtonState);
    buttonConnectToServer->setFocus(); //in case this slot was triggered programmatically
}

void NetworkConnectionWidget::lineEditServerNameEdited(const QString &szConnectionName)
{
    if (szConnectionName.isEmpty() == true) {
        lineEditServerIpV4->clear();
        lineEditServerPort->clear();
    }

    qDebug() << myNetworkAddressesManagerWidget->getMatches(szConnectionName, NetworkAddressesEnum::COLUMN_ADDRESS_NAME, Qt::MatchContains);
}

void NetworkConnectionWidget::lineEditServerIpV4Edited(const QString &szServerIpV4)
{
    lineEditServerName->clear(); //NOTE may cause problems if clearing when not wanted. Works so far

    qDebug() << myNetworkAddressesManagerWidget->getMatches(szServerIpV4, NetworkAddressesEnum::COLUMN_SERVER_IP, Qt::MatchContains);
}

void NetworkConnectionWidget::lineEditServerPortEdited(const QString &szServerPort)
{
    qDebug() << myNetworkAddressesManagerWidget->getMatches(szServerPort, NetworkAddressesEnum::COLUMN_SERVER_PORT, Qt::MatchContains);
}

void NetworkConnectionWidget::connectionRequested(const QString &szAddressName)
{
    if (szAddressName.isEmpty() == true) {
        return;
    }

    completerOptionChosen(szAddressName, NetworkAddressesEnum::COLUMN_ADDRESS_NAME);

    buttonConnectToServer->setChecked(true);
}

void NetworkConnectionWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    lineEditServerName->setFocus();
}

void NetworkConnectionWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

//    QFontMetrics fm(lineEditServerPort->fontMetrics());

//    QString szTxt(QStringLiteral("99999"));
//    QRect boundingRect = fm.boundingRect(szTxt);

//    int nMargin = lineEditServerPort->contentsMargins().left() + lineEditServerPort->contentsMargins().right();

//    lineEditServerPort->setFixedWidth(lineEditServerPort->contentsRect().width() / 2);

}

