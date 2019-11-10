#pragma once

#include <QDebug>

#include "data/StandardItemModel.h"

class LoggerTableProxyModel;

namespace NetworkAddressesEnum
{

enum Columns {
    COLUMN_ADDRESS_NAME,
    COLUMN_SERVER_IP,
    COLUMN_SERVER_PORT,
    COUNT_TABLE_COLUMNS
};

} // namespace NetworkAddressesEnum

class NetworkAddressesModel : public StandardItemModel
{
    Q_OBJECT

public:
    explicit NetworkAddressesModel(QObject *parent = nullptr);
    ~NetworkAddressesModel() override;

protected:
    static QString getColumnName(const NetworkAddressesEnum::Columns eColumn);

private:
    void setupHeader();

//reimplemented methods
public:
    QVariant data(const QModelIndex &index, int role) const override;

};


