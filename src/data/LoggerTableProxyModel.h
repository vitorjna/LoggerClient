#pragma once

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QSortFilterProxyModel>
#include <QStandardItem>
#include <QThread>

#include "application/GlobalConstants.h"
#include "interface/IntMutexable.h"
#include "ui/ToastNotificationWidget.h"

class LoggerTableItemModel;

namespace LoggerEnum
{

enum LoggerSeverity {
    TRACE,
    DEBUG_L,
    INFO,
    WARNING,
    ERROR_L,
    FATAL,
//    NONE,
    COUNT_LOGGER_SEVERITY
};

enum LoggerPattern {
    PATTERN_TIMESTAMP,
    PATTERN_CLASS,
    PATTERN_SEVERITY,
    PATTERN_MESSAGE,
    PATTERN_THREADID,
    COUNT_LOGGER_PATTERN
}; // https://logging.apache.org/log4j/1.2/apidocs/org/apache/log4j/EnhancedPatternLayout.html

enum Columns {
    COLUMN_INDEX,
    COLUMN_TIMESTAMP,
    COLUMN_THREADID,
    COLUMN_CLASS,
    COLUMN_SEVERITY,
    COLUMN_MESSAGE,
    COUNT_TABLE_COLUMNS
};

}  // namespace LoggerEnum

class LoggerTableProxyModel : public QSortFilterProxyModel, public IntMutexable
{
    Q_OBJECT

public:
    explicit LoggerTableProxyModel(QObject *parent = nullptr);
    ~LoggerTableProxyModel() override = default;

    static int getLogSeverityFromName(const QString &szSeverity);

    void appendRow(const QString &szRowData, bool bAppendToRawData = true);
    void appendRows(const QStringList &szaRowsData, bool bAppendToRawData = true);

    QList<QAction *> *generateActionsForIndex(const QModelIndex &myModelIndex, QWidget *parent);

    void setFilterRegExp(const QRegExp &regExp, bool bActualFilter = true);
    void reApplyFilter();

    int getVisibleIndexForColumn(const LoggerEnum::Columns eColumn);
    LoggerEnum::Columns getColumnForVisibleIndex(const int nIndex);

    void resetIndex();

protected:
    static QString getColumnName(const LoggerEnum::Columns eColumn);
    static void fillLoggerPatternElements();
    static void fillLoggerSeverityNames();
    QList<QStandardItem *> parseLogMessage(const QString &szRowData);

private:
    struct patternData {
        LoggerEnum::LoggerPattern eCurrentPattern{LoggerEnum::COUNT_LOGGER_PATTERN};    // current pattern
        int nDataStartOffset{-1};                                                       // offset for start index (to account for extra separators between elements)
        QChar cEndSeparator{'\0'};                                                      // char of the end separator for this pattern. For the last group, separator is '\0' (to the end of the message)

        void print() {
            qDebug() << "patternData: "
                     << "\tpattern:"    << szaLoggerPatternElements[eCurrentPattern]
                     << "\toffset:"     << nDataStartOffset
                     << "\tseparator:"  << cEndSeparator;
        }
    };

    void createNewItemModel(bool bSetAsModel = false);

    void updateLoggerPatternCache();

    void reparseTableData();

    /// Extract elements from message received
    /// If bIgnoreExtraSeparators is true, nExtraSeparatorsOffset will have the offset for the next index, so the extra separators at the end of this pattern are ignored
    /// Otherwise, if multiple separators appear together, they would count as: [separator][message (but still separator)][separator]
    /// This is true if the separator is space. Otherwise, this logic doesn't add up
    /// Ex:
    ///     Log: 2018-09-05 11:45:34.783    32f0 DataStoreEvent:132    TRACE       EventStore create
    ///     All the extra spaces are ignored, and the offset will allow the next pattern to move to the start of the actual text
    ///
    /// Generic: for pattern elements without any special parsing needs
    /// Timestamp: to parse the timestamp correctly, as there is a space and other characters between the timestamp elements
    int getEndIndexGeneric      (const QString &szRowData, int nStartIndex, const QChar cEndSeparator, bool bIgnoreExtraSeparators, int &nExtraSeparatorsOffset) const;
    int getEndIndexTimestamp    (const QString &szRowData, int nStartIndex, const QChar cEndSeparator, bool bIgnoreExtraSeparators, int &nExtraSeparatorsOffset) const;

    static QVector<QString>     szaLoggerPatternElements;
    static QVector<QString>     szaLoggerSeverityNames;

    static const QChar          SEPARATOR_END;

    LoggerTableItemModel        *myItemModel;
    uint32_t                    nRowIndexCount;
    QString                     szLoggerPattern;
    QVector<patternData>        naLoggerPatternData;
    QVector<int>                naColumnOrder;

    QStringList                 szaTableModelRaw;
    QRegExp                     myFilter;

public slots:
    void clear();

    void parseFile(const QString &szFilename);
    void parseClipboard();

    void newMessageReceived(const QString &szMessage);
    void setLoggerPattern(const QString &szPattern);

    void menuActionClickedFilter(bool bState);
    void menuActionClickedOpenFile(bool bState);

    void deleteRowsAbove(bool bState);
    void deleteRowsBelow(bool bState);

signals:
    void clipboardParsingResult(const GlobalConstants::ErrorCode eParsingResult);
    void fileParsingResult(const int nResult, const QString &szFilename = QLatin1String(""));
    void filterStateChanged(bool);

    void showNotification(const QString &szMessage, const ToastNotificationWidget::NotificationType eNotifType, const int nTimeoutMs);

};

