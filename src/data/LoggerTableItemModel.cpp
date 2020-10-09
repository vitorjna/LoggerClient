#include "LoggerTableItemModel.h"
#include "application/AppSettings.h"
#include "data/LoggerTableProxyModel.h"

LoggerTableItemModel::LoggerTableItemModel(LoggerTableProxyModel *myProxyModel, QObject *parent)
    : QStandardItemModel(parent)
    , myProxyModel(myProxyModel)
    , szaKeywords(AppSettings::getValue(AppSettings::KEY_KEYWORDS_HIGHLIGHT, AppSettings::getDefaultValue(AppSettings::KEY_KEYWORDS_HIGHLIGHT)).toStringList())
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
    switch (static_cast<LoggerEnum::LoggerSeverity>(LoggerTableProxyModel::getLogSeverityFromName(szLevel.trimmed().toUpper()))) {
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

//        case LoggerEnum::NONE:
        case LoggerEnum::COUNT_LOGGER_SEVERITY:
            return myColorUnknown;
    }
}

void LoggerTableItemModel::updateKeywords(const QStringList &szaKeywords)
{
    this->szaKeywords.clear();

    for (const QString &szKeyword : szaKeywords) {
        if (szKeyword.trimmed().size() != 0) {
            this->szaKeywords.push_back(szKeyword.trimmed());
        }
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

                if (szaKeywords.size() != 0) {
                    int nColPosClass    = myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_CLASS);
                    int nColPosThreadId = myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_THREADID);
                    int nColPosSeverity = myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_SEVERITY);
                    int nColPosMessage  = myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_MESSAGE);

                    if (nColPosClass != -1
                        || nColPosThreadId != -1
                        || nColPosSeverity != -1
                        || nColPosMessage != -1) {

                        const QString &szClass      = nColPosClass    == -1 ? "" : this->item(index.row(), nColPosClass)->text();
                        const QString &szThreadId   = nColPosThreadId == -1 ? "" : this->item(index.row(), nColPosThreadId)->text();
                        const QString &szSeverity   = nColPosSeverity == -1 ? "" : this->item(index.row(), nColPosSeverity)->text();
                        const QString &szMessage    = nColPosMessage  == -1 ? "" : this->item(index.row(), nColPosMessage)->text();

                        for (const QString &szKeyword : szaKeywords) {
                            if (szClass.contains(szKeyword)
                                || szThreadId.contains(szKeyword)
                                || szSeverity.contains(szKeyword)
                                || szMessage.contains(szKeyword)) {

                                static const QBrush myBackgroundColorHighlight(QColor(255, 64, 64, 224));
                                return myBackgroundColorHighlight;
                            }
                        }
                    }
                }

                int nColumnLevelPos = myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_SEVERITY);

                if (nColumnLevelPos != -1) {
                    const QStandardItem *myStandardItem = this->item(index.row(), nColumnLevelPos);

                    return QBrush(getColorForLevel(myStandardItem->text()));
                }
            }

            static const QBrush myBackgroundColor(QColor(32, 32, 32));
            return myBackgroundColor;

        case Qt::ForegroundRole:
            static const QBrush myForegroundColor(QColor(224, 224, 224));
            return myForegroundColor;

        default:
            return QStandardItemModel::data(index, role);
    }

    return QStandardItemModel::data(index, role);
}

