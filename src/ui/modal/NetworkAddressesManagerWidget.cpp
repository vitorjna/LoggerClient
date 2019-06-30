#include "NetworkAddressesManagerWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "ui/ToastNotificationWidget.h"
#include "util/NetworkUtils.h"
#include "view/StandardItemView.h"

NetworkAddressesManagerWidget::NetworkAddressesManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();
    loadSettings();
}

NetworkAddressesManagerWidget::~NetworkAddressesManagerWidget() = default;

QStringList NetworkAddressesManagerWidget::getRow(const QString &szFind, const NetworkAddressesEnum::Columns eColumn)
{
    QList<QStandardItem *> myMatches = tableModelAddresses->findItems(szFind, Qt::MatchStartsWith, eColumn);

    if (myMatches.isEmpty() == true) {
        return QStringList();
    }

    int nRow = myMatches.at(0)->row();

    QStringList szaMatches;
    szaMatches.reserve(NetworkAddressesEnum::COUNT_TABLE_COLUMNS);

    for (int nCol = 0; nCol < NetworkAddressesEnum::COUNT_TABLE_COLUMNS; ++nCol) {
        szaMatches.append(tableModelAddresses->item(nRow, nCol)->text());
    }

    return szaMatches;
}

QStringList NetworkAddressesManagerWidget::getMatches(const QString &szFind, const NetworkAddressesEnum::Columns eColumn)
{
    QList<QStandardItem *> myMatches = tableModelAddresses->findItems(szFind, Qt::MatchStartsWith, eColumn);

    if (myMatches.isEmpty() == true) {
        return QStringList();
    }

    QStringList szaMatches;
    szaMatches.reserve(myMatches.size());

    for (const QStandardItem *myMatch : myMatches) {
        szaMatches.append(myMatch->text());
    }

    return szaMatches;
}

void NetworkAddressesManagerWidget::setValues(const QString &szAddressName, const QString &szServerIpV4, const QString &szServerPort)
{
    lineEditServerName->setText(szAddressName);
    lineEditServerIpV4->setText(szServerIpV4);
    lineEditServerPort->setText(szServerPort);
}

void NetworkAddressesManagerWidget::pushButtonSaveAddressPushed(bool checked)
{
    Q_UNUSED(checked)

    const QString szAddressName = lineEditServerName->text();
    const QString szServerIpV4  = lineEditServerIpV4->text();
    const QString szServerPort  = lineEditServerPort->text();

    if (szAddressName.isEmpty() == true
        || szServerIpV4.isEmpty() == true
        || szServerPort.isEmpty() == true) {

        ToastNotificationWidget::showMessage(this, tr("Please fill all fields"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    if (NetworkUtils::isIpV4Address(szServerIpV4) == false) {
        ToastNotificationWidget::showMessage(this, tr("Invalid IP Address"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    if (NetworkUtils::isValidPort(szServerPort) == false) {
        ToastNotificationWidget::showMessage(this, tr("Invalid IP Port"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    int nRow = tableModelAddresses->indexOf(szAddressName, NetworkAddressesEnum::COLUMN_ADDRESS_NAME);

    //found entry with same name
    if (nRow != -1) {
        QMessageBox::StandardButton eReplaceItem = QMessageBox::question(this, tr("Replace address"), tr("Address name already exists, replace IP and Port?"), QMessageBox::Yes | QMessageBox::No);

        if (eReplaceItem == QMessageBox::Yes) {
            tableModelAddresses->item(nRow, NetworkAddressesEnum::COLUMN_SERVER_IP)->setText(szServerIpV4);
            tableModelAddresses->item(nRow, NetworkAddressesEnum::COLUMN_SERVER_PORT)->setText(szServerPort);

            tableModelAddresses->sort();
        }

        return;
    }

    nRow = tableModelAddresses->indexOf(szServerIpV4, NetworkAddressesEnum::COLUMN_SERVER_IP);

    //found entry with same IP
    if (nRow != -1) {
        //entry also has the same Port
        if (tableModelAddresses->item(nRow, NetworkAddressesEnum::COLUMN_SERVER_PORT)->text() == szServerPort) {

            QMessageBox::StandardButton eReplaceItem = QMessageBox::question(this, tr("Replace IP and Port"), tr("IP and Port combination already exists, replace name?"), QMessageBox::Yes | QMessageBox::No);

            if (eReplaceItem == QMessageBox::Yes) {
                tableModelAddresses->item(nRow, NetworkAddressesEnum::COLUMN_ADDRESS_NAME)->setText(szAddressName);

                tableModelAddresses->sort();
            }

            return;
        }
    }

    QList<QStandardItem *> myTableRow;

    myTableRow.append(new QStandardItem(szAddressName));
    myTableRow.append(new QStandardItem(szServerIpV4));
    myTableRow.append(new QStandardItem(szServerPort));

    tableModelAddresses->appendRow(myTableRow);
}

void NetworkAddressesManagerWidget::customContextMenuRequestedOnTableView(const QPoint &pos)
{
    QModelIndex myModelIndex = tableViewAddresses->indexAt(pos);

    int nRow = myModelIndex.row();

    QList<QAction *> *myActionList = new QList<QAction *>();

    //TODO only one action for now. If more are needed, do as in the main window

    const QString szAddressName = tableModelAddresses->item(nRow, NetworkAddressesEnum::COLUMN_ADDRESS_NAME)->text();
    const QString szServerIpV4  = tableModelAddresses->item(nRow, NetworkAddressesEnum::COLUMN_SERVER_IP)->text();
    const QString szServerPort  = tableModelAddresses->item(nRow, NetworkAddressesEnum::COLUMN_SERVER_PORT)->text();

    QAction *myActionConnect = new QAction(this);
    myActionConnect->setText(tr("Connect to \"") + szAddressName + QStringLiteral("\" (") + szServerIpV4 + ':' + szServerPort + ')');
    myActionConnect->setData(szAddressName);

    connect(myActionConnect, &QAction::triggered, this, &NetworkAddressesManagerWidget::menuActionClickedConnect);

    myActionList->push_back(myActionConnect);

    QMenu *myMenu = new QMenu(this);
    myMenu->addActions(*myActionList);
    myMenu->popup(tableViewAddresses->viewport()->mapToGlobal(pos));
}

void NetworkAddressesManagerWidget::menuActionClickedConnect(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    const QString szAddressName = mySenderAction->data().toString();

    emit connectionRequested(szAddressName);
}

void NetworkAddressesManagerWidget::setupUi()
{
    int nCurrentRow = 0;

    QGridLayout *myMainLayout = new QGridLayout(this);
    myMainLayout->setContentsMargins(10, 10, 10, 10);
    myMainLayout->setSpacing(20);

    QHBoxLayout *myEditsLayout = new QHBoxLayout();
    {
        lineEditServerName = new QLineEdit(this);
        lineEditServerName->setPlaceholderText(tr("Address Name"));
        lineEditServerName->setToolTip(tr("Address Name"));

        lineEditServerIpV4 = new QLineEdit(this);
        lineEditServerIpV4->setPlaceholderText(tr("Server IP"));
        lineEditServerIpV4->setToolTip(tr("Server IP"));

        lineEditServerPort = new QLineEdit(this);
        lineEditServerPort->setPlaceholderText(tr("Port"));
        lineEditServerPort->setToolTip(tr("Port"));

        pushButtonSaveAddress = new QPushButton();
        pushButtonSaveAddress->setCheckable(false);
        pushButtonSaveAddress->setText(tr("Save"));

        connect(lineEditServerName, SIGNAL(returnPressed()), pushButtonSaveAddress, SLOT(click()));
        connect(lineEditServerIpV4, SIGNAL(returnPressed()), pushButtonSaveAddress, SLOT(click()));
        connect(lineEditServerPort, SIGNAL(returnPressed()), pushButtonSaveAddress, SLOT(click()));


        myEditsLayout->addWidget(lineEditServerName);
        myEditsLayout->addWidget(lineEditServerIpV4);
        myEditsLayout->addWidget(lineEditServerPort);
        myEditsLayout->addWidget(pushButtonSaveAddress);
    }

    {
        tableModelAddresses = new NetworkAddressesModel(this);
        tableModelAddresses->setAllowDuplicates(false);
        tableModelAddresses->setSortColumn(NetworkAddressesEnum::COLUMN_ADDRESS_NAME);

        tableViewAddresses = new StandardItemView(this);
        tableViewAddresses->setAlternatingRowColors(true);
        tableViewAddresses->setModel(tableModelAddresses);
        tableViewAddresses->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        tableViewAddresses->setSortingEnabled(true);
        tableViewAddresses->setShowGrid(true);
        tableViewAddresses->setContextMenuPolicy(Qt::CustomContextMenu);
        tableViewAddresses->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tableViewAddresses->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableViewAddresses->setWordWrap(false);

        //Horizontal
        tableViewAddresses->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableViewAddresses->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableViewAddresses->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableViewAddresses->horizontalHeader()->setHighlightSections(false);
        tableViewAddresses->horizontalHeader()->setStretchLastSection(true);
        //Vertical
        tableViewAddresses->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableViewAddresses->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableViewAddresses->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        tableViewAddresses->verticalHeader()->setVisible(false);
    }

    myMainLayout->addLayout(myEditsLayout, nCurrentRow, 0);
    ++nCurrentRow;
    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
    myMainLayout->addWidget(tableViewAddresses, nCurrentRow, 0, 2, -1);

    this->setLayout(myMainLayout);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window  | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    this->resize(800, 600); //TODO doesn't work with large font sizes

    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));
    this->setWindowTitle(tr("Network Address Manager"));

    this->hide();
}

void NetworkAddressesManagerWidget::setupSignalsAndSlots()
{
    connect(this,                                   &NetworkAddressesManagerWidget::connectionRequested,
            this,                                   [ = ] { this->hide(); });

    connect(pushButtonSaveAddress,                  &QPushButton::clicked,
            this,                                   &NetworkAddressesManagerWidget::pushButtonSaveAddressPushed);

    connect(tableViewAddresses,                     &StandardItemView::customContextMenuRequested,
            this,                                   &NetworkAddressesManagerWidget::customContextMenuRequestedOnTableView);

    connect(tableViewAddresses,                     &StandardItemView::activated,
            this,
    [ = ] (const QModelIndex & index) {
        if (index.isValid() == true) {
            emit connectionRequested(tableModelAddresses->item(index.row(), NetworkAddressesEnum::COLUMN_ADDRESS_NAME)->text());
        }
    });
}

void NetworkAddressesManagerWidget::loadSettings()
{
    const QString szNetworkAddresses = AppSettings::getValue(AppSettings::KEY_LOGGER_SERVER_ADDRESSES).toString();

    if (szNetworkAddresses.isEmpty() == false) {

        const QStringList szaNetworkAddresses = szNetworkAddresses.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

        for (int nRow = 0; nRow < szaNetworkAddresses.size(); ++nRow) {

            const QStringList szaNetworkAddress = szaNetworkAddresses.at(nRow).split(GlobalConstants::SEPARATOR_SETTINGS_LIST_2);

            if (szaNetworkAddress.size() != NetworkAddressesEnum::COUNT_TABLE_COLUMNS) {
                ToastNotificationWidget::showMessage(this, tr("Could not parse address: ") + szaNetworkAddress.join(GlobalConstants::SEPARATOR_SETTINGS_LIST_2), ToastNotificationWidget::ERROR, 3000);
                continue;
            }

            QList<QStandardItem *> myTableRow;

            for (int nCol = 0; nCol < NetworkAddressesEnum::COUNT_TABLE_COLUMNS; ++nCol) {
                myTableRow.append(new QStandardItem(szaNetworkAddress.at(nCol)));
            }

            tableModelAddresses->appendRow(myTableRow);
        }
    }
}

void NetworkAddressesManagerWidget::saveSettings()
{
    QStringList szaNetworkAddresses;
    szaNetworkAddresses.reserve(tableModelAddresses->rowCount());

    for (int nRow = 0; nRow < tableModelAddresses->rowCount(); ++nRow) {
        QStringList szaNetworkAddress;

        for (int nColumn = 0; nColumn < NetworkAddressesEnum::COUNT_TABLE_COLUMNS; ++nColumn) {
            QString szText = tableModelAddresses->item(nRow, nColumn)->text();

            if (szText.trimmed().isEmpty() == true) {
                break;
            }

            szaNetworkAddress.append(szText);
        }

        if (szaNetworkAddress.size() != NetworkAddressesEnum::COUNT_TABLE_COLUMNS) {
            continue;
        }

        szaNetworkAddresses.append(szaNetworkAddress.join(GlobalConstants::SEPARATOR_SETTINGS_LIST_2));
    }

    AppSettings::setValue(AppSettings::KEY_LOGGER_SERVER_ADDRESSES, szaNetworkAddresses.join(GlobalConstants::SEPARATOR_SETTINGS_LIST));
}

void NetworkAddressesManagerWidget::hideEvent(QHideEvent *event)
{
    emit aboutToHide();

    QWidget::hideEvent(event);

    saveSettings();

    lineEditServerName->clear();
    lineEditServerIpV4->clear();
    lineEditServerPort->clear();
}

void NetworkAddressesManagerWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToHide();

    QWidget::closeEvent(event);
}

