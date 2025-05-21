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

QColor LoggerTableItemModel::getColorForLevel(const QString &szLevel)
{
    //    static int nLevels = {0, 32, 64, 96, 128, 160, 192, 224, 255};

    //TODO place colors in a class, as statics
    static const QColor myColorTrace  (0, 64, 64, 192);
    static const QColor myColorDebug  (0, 96, 96, 255);
    static const QColor myColorInfo   (128, 128, 0, 192);
    static const QColor myColorWarning(128, 128, 0, 255);
    static const QColor myColorError  (128, 0, 0, 192);
    static const QColor myColorFatal  (128, 0, 0, 255);
    static const QColor myColorUnknown(Qt::gray);

    switch (LoggerTableProxyModel::getLogSeverityFromName(szLevel)) {
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

        case Qt::BackgroundRole: {
            if (this->columnCount() > 1) {

                if (szaKeywords.empty() == false) {
                    static const QList<LoggerEnum::Columns> naColsToCheck = { LoggerEnum::COLUMN_CLASS,
                                                                              LoggerEnum::COLUMN_THREADID,
                                                                              LoggerEnum::COLUMN_SEVERITY,
                                                                              LoggerEnum::COLUMN_MESSAGE
                                                                            };

                    for (const LoggerEnum::Columns eCol : naColsToCheck) {
                        const qsizetype nColPos = myProxyModel->getVisibleIndexForColumn(eCol);

                        if (nColPos != -1) {
                            const QString &szText = this->item(index.row(), nColPos)->text();

                            for (const QString &szKeyword : szaKeywords) {
                                if (szText.contains(szKeyword)) {
                                    static const QBrush myBackgroundColorHighlight(QColor(255, 64, 64, 192));
                                    return myBackgroundColorHighlight;
                                }
                            }
                        }
                    }
                }

                const qsizetype nColumnLevelPos = myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_SEVERITY);

                if (nColumnLevelPos != -1) {
                    return QBrush(getColorForLevel(this->item(index.row(), nColumnLevelPos)->text()));
                }
            }

            static const QBrush myBackgroundColor(QColor(32, 32, 32));
            return myBackgroundColor;
        }

        case Qt::ForegroundRole: {
            static const QBrush myForegroundColor(QColor(224, 224, 224));
            return myForegroundColor;
        }

        default:
            return QStandardItemModel::data(index, role);
    }
}

