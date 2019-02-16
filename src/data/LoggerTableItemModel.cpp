#include "LoggerTableItemModel.h"
#include "data/LoggerTableProxyModel.h"

LoggerTableItemModel::LoggerTableItemModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

LoggerTableItemModel::~LoggerTableItemModel()
{
}

QVariant LoggerTableItemModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        if (index.column() == LoggerEnum::COLUMN_THREADID) {
            return QVariant(Qt::AlignTop | Qt::AlignHCenter);

        } else {
            return QVariant(Qt::AlignTop | Qt::AlignLeft);
        }

    } else if (role == Qt::BackgroundRole) {
        if (this->columnCount() > 1) { //TODO change to is column visible
            if (this->item(index.row(), LoggerEnum::COLUMN_CLASS)->text().contains(QLatin1String("LoggerCppDK"))) {
                return QBrush(QColor(255, 64, 64, 224));

            } else {
                QStandardItem *myStandardItem = this->item(index.row(), LoggerEnum::COLUMN_SEVERITY);

                QColor color = LoggerTableProxyModel::calculateColorForLevel(myStandardItem->text());
                return QBrush(color);
            }
        }

    } else if (role == Qt::ForegroundRole) {
        QColor color(224, 224, 224);

        return QBrush(color);
    }

    return QStandardItemModel::data(index, role);
}

