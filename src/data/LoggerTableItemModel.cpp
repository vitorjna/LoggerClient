#include "LoggerTableItemModel.h"
#include "data/LoggerTableProxyModel.h"

LoggerTableItemModel::LoggerTableItemModel(LoggerTableProxyModel *myProxyModel, QObject *parent)
    : QStandardItemModel(parent)
    , myProxyModel(myProxyModel)
{
}

LoggerTableItemModel::~LoggerTableItemModel() = default;

QColor LoggerTableItemModel::getColorForLevel(const QString &szLevel) const
{
    //    static int nLevels = {0, 32, 64, 96, 128, 160, 192, 224, 255};

    static const QColor myColorTrace  (0, 64, 64, 192);
    static const QColor myColorDebug  (0, 96, 96, 255);
    static const QColor myColorInfo   (128, 128, 0, 192);
    static const QColor myColorWarning(128, 128, 0, 255);
    static const QColor myColorError  (128, 0, 0, 192);
    static const QColor myColorFatal  (128, 0, 0, 255);
    static const QColor myColorUnknown(Qt::gray);

    //TODO place colors in a class, as statics
    switch (static_cast<LoggerEnum::LoggerSeverity>(LoggerTableProxyModel::getLogSeverityFromName(szLevel))) {
        case LoggerEnum::TRACE:
            return myColorTrace;

        case LoggerEnum::DEBUG_L:
            return myColorDebug;

        case LoggerEnum::INFO:
            return myColorInfo;

        case LoggerEnum::WARNING:
            return myColorWarning;

        case LoggerEnum::ERROR_L:
            return myColorError;

        case LoggerEnum::FATAL:
            return myColorFatal;

        case LoggerEnum::NONE:
        case LoggerEnum::COUNT_LOGGER_SEVERITY:
        default:
            return myColorUnknown;
    }
}

QVariant LoggerTableItemModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::TextAlignmentRole:
            if (index.column() == myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_THREADID)) {
                static const QVariant myAlignTopCenter(Qt::AlignVCenter | Qt::AlignHCenter);
                return myAlignTopCenter;

            } else {
                static const QVariant myAlignTopLeft(Qt::AlignVCenter | Qt::AlignLeft);
                return myAlignTopLeft;
            }

        case Qt::BackgroundRole:
            if (this->columnCount() > 1) {
                if (this->item(index.row(), myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_CLASS))->text().startsWith(QLatin1String("LoggerCppDK"))) {
                    static const QBrush myBackgroundColorHighlight(QColor(255, 64, 64, 224));
                    return myBackgroundColorHighlight;

                } else {
                    QStandardItem *myStandardItem = this->item(index.row(), myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_SEVERITY));

                    const QColor color = getColorForLevel(myStandardItem->text());
                    return QBrush(color);
                }
            }

            break;

        case Qt::ForegroundRole:
            static const QBrush myForegroundColor(QColor(224, 224, 224));
            return myForegroundColor;

        default:
            return QStandardItemModel::data(index, role);
    }

    return QStandardItemModel::data(index, role);
}

