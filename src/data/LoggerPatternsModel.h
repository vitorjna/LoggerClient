#pragma once

#include <QDebug>

#include "data/StandardItemModel.h"

class LoggerTableProxyModel;

namespace LoggerPatternsEnum
{

enum Columns {
    COLUMN_PATTERN_NAME,
    COLUMN_PATTERN,
    COUNT_TABLE_COLUMNS
};

} // namespace LoggerPatternsEnum

class LoggerPatternsModel : public StandardItemModel
{
    Q_OBJECT

public:
    explicit LoggerPatternsModel(QObject *parent = nullptr);
    ~LoggerPatternsModel() override;

protected:
    static QString getColumnName(const LoggerPatternsEnum::Columns eColumn);

private:
    void setupHeader();

//reimplemented methods
public:
    QVariant data(const QModelIndex &index, int role) const override;

};


