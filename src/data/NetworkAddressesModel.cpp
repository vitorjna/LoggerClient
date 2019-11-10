#include "NetworkAddressesModel.h"
#include "util/NetworkUtils.h"

NetworkAddressesModel::NetworkAddressesModel(QObject *parent)
    : StandardItemModel(parent)
{
    setupHeader();
}

NetworkAddressesModel::~NetworkAddressesModel() = default;

QString NetworkAddressesModel::getColumnName(const NetworkAddressesEnum::Columns eColumn)
{
    switch (eColumn) {
        case NetworkAddressesEnum::COLUMN_ADDRESS_NAME:
            return tr("Address Name");

        case NetworkAddressesEnum::COLUMN_SERVER_IP:
            return tr("Server IP");

        case NetworkAddressesEnum::COLUMN_SERVER_PORT:
            return tr("Port");

        case NetworkAddressesEnum::COUNT_TABLE_COLUMNS:
            return QLatin1String("");
    }
}

void NetworkAddressesModel::setupHeader()
{
    this->setColumnCount(NetworkAddressesEnum::COUNT_TABLE_COLUMNS);

    for (int nIndex = 0; nIndex < NetworkAddressesEnum::COUNT_TABLE_COLUMNS; ++nIndex) {
        this->setHeaderData(nIndex,
                            Qt::Horizontal,
                            getColumnName(static_cast<NetworkAddressesEnum::Columns>(nIndex)),
                            Qt::DisplayRole);
    }
}

QVariant NetworkAddressesModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::TextAlignmentRole:
            if (index.column() == NetworkAddressesEnum::COLUMN_SERVER_IP
                || index.column() == NetworkAddressesEnum::COLUMN_SERVER_PORT) {

                return Qt::AlignCenter;

            } else {
                static const QVariant myAlignCenterLeft(Qt::AlignVCenter | Qt::AlignLeft);
                return myAlignCenterLeft;
            }

        case Qt::BackgroundRole: {
            switch (static_cast<NetworkAddressesEnum::Columns>(index.column())) {
                case NetworkAddressesEnum::COLUMN_ADDRESS_NAME:
                    break;

                case NetworkAddressesEnum::COLUMN_SERVER_IP:
                    if (NetworkUtils::isIpV4Address(this->item(index.row(), NetworkAddressesEnum::COLUMN_SERVER_IP)->text()) == false) {
                        return myBrushError;
                    }

                    break;

                case NetworkAddressesEnum::COLUMN_SERVER_PORT:
                    if (NetworkUtils::isValidPort(this->item(index.row(), NetworkAddressesEnum::COLUMN_SERVER_PORT)->text()) == false) {
                        return myBrushError;
                    }

                    break;

                case NetworkAddressesEnum::COUNT_TABLE_COLUMNS:
                    break;
            }

            break;
        }

        default:
            return StandardItemModel::data(index, role);
    }

    return StandardItemModel::data(index, role);
}

