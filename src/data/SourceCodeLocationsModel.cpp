#include "SourceCodeLocationsModel.h"
#include "util/NetworkUtils.h"

SourceCodeLocationsModel::SourceCodeLocationsModel(QObject *parent)
    : StandardItemModel(parent)
{
    setupHeader();
}

SourceCodeLocationsModel::~SourceCodeLocationsModel() = default;

QString SourceCodeLocationsModel::getColumnName(const SourceCodeLocationsEnum::Columns eColumn)
{
    switch (eColumn) {
        case SourceCodeLocationsEnum::COLUMN_PROJECT_NAME:
            return tr("Project");

        case SourceCodeLocationsEnum::COLUMN_SOURCE_PATH:
            return tr("Path");

        case SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS:
            return QString();
    }
}

void SourceCodeLocationsModel::setupHeader()
{
    this->setColumnCount(SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS);

    for (int nIndex = 0; nIndex < SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS; ++nIndex) {
        this->setHeaderData(nIndex,
                            Qt::Horizontal,
                            getColumnName(static_cast<SourceCodeLocationsEnum::Columns>(nIndex)),
                            Qt::DisplayRole);
    }
}

QVariant SourceCodeLocationsModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::TextAlignmentRole:
            if (index.column() == SourceCodeLocationsEnum::COLUMN_PROJECT_NAME) {
                return Qt::AlignCenter;

            } else {
                static const QVariant myAlignCenterLeft(Qt::AlignVCenter | Qt::AlignLeft);
                return myAlignCenterLeft;
            }

//        case Qt::BackgroundRole: {
//            switch (static_cast<SourceCodeLocationsEnum::Columns>(index.column())) {
//                case NetworkAddressesEnum::COLUMN_ADDRESS_NAME:
//                    break;

//                case NetworkAddressesEnum::COLUMN_SERVER_IP:
//                    if (NetworkUtils::isIpV4Address(this->item(index.row(), NetworkAddressesEnum::COLUMN_SERVER_IP)->text()) == false) {
//                        return myBrushError;
//                    }

//                    break;

//                case NetworkAddressesEnum::COLUMN_SERVER_PORT:
//                    if (NetworkUtils::isValidPort(this->item(index.row(), NetworkAddressesEnum::COLUMN_SERVER_PORT)->text()) == false) {
//                        return myBrushError;
//                    }

//                    break;

//                case SourceCodeLocationsEnum::COUNT_TABLE_COLUMNS:
//                    break;
//            }

//            break;
//        }

        default:
            return StandardItemModel::data(index, role);
    }
}

