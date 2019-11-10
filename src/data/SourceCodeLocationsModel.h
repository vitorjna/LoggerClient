#pragma once

#include <QDebug>

#include "data/StandardItemModel.h"

class LoggerTableProxyModel;

namespace SourceCodeLocationsEnum
{

enum Columns {
    COLUMN_PROJECT_NAME,
    COLUMN_SOURCE_PATH,
    COUNT_TABLE_COLUMNS
};

} // namespace SourceCodeLocationsEnum

class SourceCodeLocationsModel : public StandardItemModel
{
    Q_OBJECT

public:
    explicit SourceCodeLocationsModel(QObject *parent = nullptr);
    ~SourceCodeLocationsModel() override;

protected:
    static QString getColumnName(const SourceCodeLocationsEnum::Columns eColumn);

private:
    void setupHeader();

//reimplemented methods
public:
    QVariant data(const QModelIndex &index, int role) const override;

};


