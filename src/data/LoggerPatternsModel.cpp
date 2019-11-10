#include "LoggerPatternsModel.h"
#include "util/NetworkUtils.h"

LoggerPatternsModel::LoggerPatternsModel(QObject *parent)
    : StandardItemModel(parent)
{
    setupHeader();
}

LoggerPatternsModel::~LoggerPatternsModel() = default;

QString LoggerPatternsModel::getColumnName(const LoggerPatternsEnum::Columns eColumn)
{
    switch (eColumn) {
        case LoggerPatternsEnum::COLUMN_PATTERN_NAME:
            return tr("Pattern Name");

        case LoggerPatternsEnum::COLUMN_PATTERN:
            return tr("Pattern");

        case LoggerPatternsEnum::COUNT_TABLE_COLUMNS:
            return QLatin1String("");
    }
}

void LoggerPatternsModel::setupHeader()
{
    this->setColumnCount(LoggerPatternsEnum::COUNT_TABLE_COLUMNS);

    for (int nIndex = 0; nIndex < LoggerPatternsEnum::COUNT_TABLE_COLUMNS; ++nIndex) {
        this->setHeaderData(nIndex,
                            Qt::Horizontal,
                            getColumnName(static_cast<LoggerPatternsEnum::Columns>(nIndex)),
                            Qt::DisplayRole);
    }
}

QVariant LoggerPatternsModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::TextAlignmentRole:
//            if (index.column() == LoggerPatternsEnum::COLUMN_PATTERN) {
//                static const QVariant myAlignTopCenter(Qt::AlignVCenter | Qt::AlignHCenter);
//                return myAlignTopCenter;

//            } else {
                static const QVariant myAlignTopLeft(Qt::AlignVCenter | Qt::AlignLeft);
                return myAlignTopLeft;
//            }

        default:
            return StandardItemModel::data(index, role);
    }
}

