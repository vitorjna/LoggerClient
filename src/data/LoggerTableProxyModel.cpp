#include "LoggerTableProxyModel.h"
#include "application/SourceCodeHandler.h"
#include "data/LoggerTableItemModel.h"
#include "data/SourceCodeLocationsModel.h"
#include "ui/LoggerClientWidget.h"
#include "ui/ToastNotificationWidget.h"
#include "util/FileUtils.h"
#include "util/TimeUtils.h"

QVector<QString> LoggerTableProxyModel::szaLoggerPatternElements;
QVector<QString> LoggerTableProxyModel::szaLoggerSeverityNames;
const QChar      LoggerTableProxyModel::SEPARATOR_END{'\0'};

LoggerTableProxyModel::LoggerTableProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , IntMutexable()
    , myItemModel(nullptr)
    , nRowIndexCount(0)
{
    szaTableModelRaw.reserve(10000);

    fillLoggerPatternElements();
    fillLoggerSeverityNames();

    createNewItemModel(true);
}

LoggerEnum::LoggerSeverity LoggerTableProxyModel::getLogSeverityFromName(const QString &szSeverity)
{
    qsizetype nIndex = szaLoggerSeverityNames.indexOf(szSeverity.trimmed().toUpper());

    if (nIndex == -1) {
        nIndex = LoggerEnum::COUNT_LOGGER_SEVERITY;
    }

    return static_cast<LoggerEnum::LoggerSeverity>(nIndex);
}

void LoggerTableProxyModel::appendRow(const QString &szRowData, bool bAppendToRawData)
{
    if (szRowData.trimmed().isEmpty() == true) {
        return;
    }

    if (szLoggerPattern.isEmpty() == true) {
        myItemModel->appendRow(new QStandardItem(szRowData));

    } else {
        const QList<QStandardItem *> myTableRow = parseLogMessage(szRowData);

        if (myTableRow.isEmpty() == false) {
            myItemModel->appendRow(myTableRow);
        }
    }

    if (bAppendToRawData) {
        szaTableModelRaw.append(szRowData);
    }
}

void LoggerTableProxyModel::appendRows(const QStringList &szaRowsData, bool bAppendToRawData)
{
    for (const QString &szRowData : szaRowsData) {
        appendRow(szRowData, bAppendToRawData);
    }
}

QList<QAction *> *LoggerTableProxyModel::generateActionsForIndex(const QModelIndex &myModelIndex, QWidget *parent)
{
    if (myModelIndex.isValid() == false) {
        return nullptr;
    }

    QList<QAction *> *myActionList = new QList<QAction *>();

    switch (getColumnForVisibleIndex(myModelIndex.column())) {
        case LoggerEnum::COLUMN_INDEX:
            break;

        case LoggerEnum::COLUMN_TIMESTAMP:
            break;

        case LoggerEnum::COLUMN_THREADID: {
            const QString szThreadId = myModelIndex.data(Qt::DisplayRole).toString();

            if (szThreadId.isEmpty() == true) {
                break;
            }

            QAction *myActionFilter = new QAction(parent);
            myActionFilter->setText(tr("Filter by thread ") + szThreadId);
            myActionFilter->setData(szThreadId + '#' + QString::number(myModelIndex.column()));

            connect(myActionFilter, SIGNAL(triggered(bool)), this, SLOT(menuActionClickedFilter(bool)));

            myActionList->push_back(myActionFilter);

            break;
        }

        case LoggerEnum::COLUMN_CLASS: {
            QString szClassName = myModelIndex.data(Qt::DisplayRole).toString();

            if (szClassName.isEmpty() == true) {
                break;
            }

            const qsizetype nIndexOfLineNumber = szClassName.indexOf(':');

            if (nIndexOfLineNumber != -1) {
                const QString szLineNumber = szClassName.mid(nIndexOfLineNumber + 1);

                szClassName = szClassName.left(nIndexOfLineNumber);

                QAction *myActionOpenFile = new QAction(parent);
                myActionOpenFile->setText(tr("Open ") + szClassName + tr(", line ") + szLineNumber);
                myActionOpenFile->setData(szClassName + '#' + szLineNumber);

                connect(myActionOpenFile, SIGNAL(triggered(bool)), this, SLOT(menuActionClickedOpenFile(bool)));

                myActionList->push_back(myActionOpenFile);
            }

            QAction *myActionFilter = new QAction(parent);
            myActionFilter->setText(tr("Filter by class ") + szClassName);
            myActionFilter->setData(szClassName + '#' + QString::number(myModelIndex.column()));

            connect(myActionFilter, SIGNAL(triggered(bool)), this, SLOT(menuActionClickedFilter(bool)));

            myActionList->push_back(myActionFilter);
            break;
        }

        case LoggerEnum::COLUMN_SEVERITY: {
            const QString szLogLevel = myModelIndex.data(Qt::DisplayRole).toString();

            if (szLogLevel.isEmpty() == true) {
                break;
            }

            QAction *myActionFilter = new QAction(parent);
            myActionFilter->setText(tr("Filter by level ") + szLogLevel);
            myActionFilter->setData(szLogLevel + '#' + QString::number(myModelIndex.column()));

            connect(myActionFilter, SIGNAL(triggered(bool)), this, SLOT(menuActionClickedFilter(bool)));

            myActionList->push_back(myActionFilter);
            break;
        }

        case LoggerEnum::COLUMN_MESSAGE: {
            const QString szMessage = myModelIndex.data(Qt::DisplayRole).toString();

            if (szMessage.isEmpty() == true) {
                break;
            }

            QAction *myActionFilter = new QAction(parent);
            myActionFilter->setText(tr("Copy message data"));
            myActionFilter->setData(myModelIndex.column());

            connect(myActionFilter, SIGNAL(triggered(bool)), this, SLOT(menuActionClickedCopySelected(bool)));

            myActionList->push_back(myActionFilter);

            break;
        }

        case LoggerEnum::COUNT_TABLE_COLUMNS:
            break;
    }

    {
        const int nLineNumber = myModelIndex.row();

        QAction *myActionDeleteAbove = new QAction(parent);
        myActionDeleteAbove->setText(tr("Delete rows above"));
        myActionDeleteAbove->setData(nLineNumber);

        connect(myActionDeleteAbove, SIGNAL(triggered(bool)), this, SLOT(deleteRowsAbove(bool)));

        myActionList->push_back(myActionDeleteAbove);

        QAction *myActionDeleteBelow = new QAction(parent);
        myActionDeleteBelow->setText(tr("Delete rows below"));
        myActionDeleteBelow->setData(nLineNumber);

        connect(myActionDeleteBelow, SIGNAL(triggered(bool)), this, SLOT(deleteRowsBelow(bool)));

        myActionList->push_back(myActionDeleteBelow);
    }

    return myActionList;
}

void LoggerTableProxyModel::setFilterRegExp(const QRegularExpression &regExp, bool bActualFilter)
{
    if (regExp.isValid() == true) {
        myFilter = regExp;
        QSortFilterProxyModel::setFilterRegularExpression(myFilter);

        Q_EMIT filterStateChanged(bActualFilter); //if search was text, always emit false (not actual filter, just text)

    } else {
        myFilter = QRegularExpression();
        QSortFilterProxyModel::setFilterRegularExpression(myFilter);

        Q_EMIT filterStateChanged(false);
    }
}

void LoggerTableProxyModel::reApplyFilter()
{
    if (myFilter.pattern().isEmpty() == false) {
        setFilterRegExp(myFilter);
    }
}

qsizetype LoggerTableProxyModel::getVisibleIndexForColumn(const LoggerEnum::Columns eColumn)
{
    return naColumnOrder.indexOf(eColumn);
}

LoggerEnum::Columns LoggerTableProxyModel::getColumnForVisibleIndex(const int nIndex)
{
    return static_cast<LoggerEnum::Columns>(naColumnOrder.at(nIndex));
}

void LoggerTableProxyModel::resetIndex()
{
    nRowIndexCount = 0;
}

double LoggerTableProxyModel::getColumnWidthBias(const LoggerEnum::Columns eColumn)
{
    switch (eColumn) {
        case LoggerEnum::COLUMN_INDEX:
        case LoggerEnum::COLUMN_TIMESTAMP:
            return 1.0;

        case LoggerEnum::COLUMN_THREADID:
            return 0.5;

        case LoggerEnum::COLUMN_CLASS:
        case LoggerEnum::COLUMN_SEVERITY:
        case LoggerEnum::COLUMN_MESSAGE:
            return 1.0;

        case LoggerEnum::COUNT_TABLE_COLUMNS:
            return 0.0;
    }
}

QString LoggerTableProxyModel::getColumnName(const LoggerEnum::Columns eColumn)
{
    switch (eColumn) {
        case LoggerEnum::COLUMN_INDEX:
            return QStringLiteral("#");

        case LoggerEnum::COLUMN_TIMESTAMP:
            return tr("Timestamp");

        case LoggerEnum::COLUMN_THREADID:
            return tr("ThreadID");

        case LoggerEnum::COLUMN_CLASS:
            return tr("Class");

        case LoggerEnum::COLUMN_SEVERITY:
            return tr("Level");

        case LoggerEnum::COLUMN_MESSAGE:
            return tr("Message");

        case LoggerEnum::COUNT_TABLE_COLUMNS:
            return QString();
    }
}

void LoggerTableProxyModel::fillLoggerPatternElements()
{
    if (szaLoggerPatternElements.isEmpty() == false) {
        return;
    }

    szaLoggerPatternElements.resize(LoggerEnum::COUNT_LOGGER_PATTERN);

    szaLoggerPatternElements[LoggerEnum::PATTERN_TIMESTAMP]     = QStringLiteral("%d");
    szaLoggerPatternElements[LoggerEnum::PATTERN_CLASS]         = QStringLiteral("%c");
    szaLoggerPatternElements[LoggerEnum::PATTERN_SEVERITY]      = QStringLiteral("%p");
    szaLoggerPatternElements[LoggerEnum::PATTERN_MESSAGE]       = QStringLiteral("%m");
    szaLoggerPatternElements[LoggerEnum::PATTERN_THREADID]      = QStringLiteral("%t");
}

void LoggerTableProxyModel::fillLoggerSeverityNames()
{
    if (szaLoggerSeverityNames.isEmpty() == false) {
        return;
    }

    szaLoggerSeverityNames.resize(LoggerEnum::COUNT_LOGGER_SEVERITY);

    szaLoggerSeverityNames[LoggerEnum::TRACE]              = QStringLiteral("TRACE");
    szaLoggerSeverityNames[LoggerEnum::DEBUG_L]            = QStringLiteral("DEBUG");
    szaLoggerSeverityNames[LoggerEnum::INFO]               = QStringLiteral("INFO");
    szaLoggerSeverityNames[LoggerEnum::WARNING]            = QStringLiteral("WARNING");
    szaLoggerSeverityNames[LoggerEnum::WARN]               = QStringLiteral("WARN");
    szaLoggerSeverityNames[LoggerEnum::ERROR_L]            = QStringLiteral("ERROR");
    szaLoggerSeverityNames[LoggerEnum::FATAL]              = QStringLiteral("FATAL");
//    szaLoggerSeverityNames[LoggerEnum::NONE]               = QStringLiteral("NONE");
}

//DONE A big optimization was done in the pattern parsing
/// After the pattern is known, the startIndex and endChar of every pattern element is known
/// Ex: timestamp always starts at posX and ends with ' ', classname always starts at posZ and ends with ']' and so on
/// These known elements can be parsed when the pattern changes, placed in a vector and just iterate the vector and do the switch case part for each column
QList<QStandardItem *> LoggerTableProxyModel::parseLogMessage(const QString &szRowData)
{
    QList<QStandardItem *> myTableRow;
    myTableRow.reserve(myItemModel->columnCount() + 1); //52 to 51ms in 8k lines

    bool bContinueFromLast = true;

    for (qsizetype nIndex = 0; nIndex < LoggerEnum::COUNT_LOGGER_SEVERITY; ++nIndex) {
        //replaced contains of Regex (\b + Level + \b) with just QString. May have false positives, but performance: 63 to 34ms, for the same workload
        if (szRowData.contains(szaLoggerSeverityNames.at(nIndex), Qt::CaseSensitive) == true) {
            bContinueFromLast = false; //one of the levels exists, create new entry
            break;
        }
    }

    if (bContinueFromLast == true) {
        myTableRow.append(new QStandardItem(QString::number(++nRowIndexCount) + '*'));

        const int nPreviousRow = myItemModel->rowCount() - 1;

        if (nPreviousRow >= 0) {
            //copy all columns except message, but check its position in the table
            for (qsizetype nIndex = 1; nIndex < getVisibleIndexForColumn(LoggerEnum::COLUMN_MESSAGE); ++nIndex) {
                const QStandardItem *myPreviousItem = myItemModel->item(nPreviousRow, static_cast<int>(nIndex));

                if (myPreviousItem != nullptr) {
                    QStandardItem *myPreviousItemClone = myPreviousItem->clone();
                    myTableRow.append(myPreviousItemClone);
                }
            }

        } else {
            //if previous line doesn't exist, add empty elements before the message
            for (qsizetype nIndex = 1; nIndex < getVisibleIndexForColumn(LoggerEnum::COLUMN_MESSAGE); ++nIndex) {
                myTableRow.append(new QStandardItem(""));
            }
        }

        myTableRow.append(new QStandardItem(szRowData));

    } else {
        myTableRow.append(new QStandardItem(QString::number(++nRowIndexCount)));

        int nDataStartIndex = 0;

        for (const patternData &patternDataElement : std::as_const(naLoggerPatternData)) {

            const LoggerEnum::LoggerPattern nCurrentPattern = patternDataElement.eCurrentPattern;
            const int nStartOffset                          = patternDataElement.nDataStartOffset;

            if (patternDataElement.cEndSeparator == SEPARATOR_END) { //last element
                myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex + nStartOffset, -1)));

            } else {
                nDataStartIndex += nStartOffset;

                qsizetype nDataEndIndex;
                int nExtraSeparatorsOffset = 0; //if multiple end separators are found, adjust the end offset. Ex: multiple spaces at the end are ignored and the end index is adjusted for the next element

                //removed trim after calling szRowData.mid. From 70 to 68 ms
                switch (nCurrentPattern) {
                    case LoggerEnum::PATTERN_TIMESTAMP: {
                        //TODO place date and time in different columns
                        nDataEndIndex = getEndIndexTimestamp(szRowData, nDataStartIndex, patternDataElement.cEndSeparator, true, nExtraSeparatorsOffset);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex)));
                        break;
                    }

                    case LoggerEnum::PATTERN_THREADID: {
                        nDataEndIndex = getEndIndexGeneric(szRowData, nDataStartIndex, patternDataElement.cEndSeparator, true, nExtraSeparatorsOffset);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex)));
                        break;
                    }

                    case LoggerEnum::PATTERN_SEVERITY: {
                        nDataEndIndex = getEndIndexGeneric(szRowData, nDataStartIndex, patternDataElement.cEndSeparator, true, nExtraSeparatorsOffset);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex)));
                        break;
                    }

                    case LoggerEnum::PATTERN_CLASS: {
                        //TODO place line number in another column
                        nDataEndIndex = getEndIndexGeneric(szRowData, nDataStartIndex, patternDataElement.cEndSeparator, true, nExtraSeparatorsOffset);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex)));
                        break;
                    }

                    case LoggerEnum::PATTERN_MESSAGE: {
                        nDataEndIndex = getEndIndexGeneric(szRowData, nDataStartIndex, patternDataElement.cEndSeparator, true, nExtraSeparatorsOffset);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex)));
                        break;
                    }

                    case LoggerEnum::COUNT_LOGGER_PATTERN:
                        nDataEndIndex = 0;
                        break;
                }

                nDataStartIndex = static_cast<int>(nDataEndIndex + nExtraSeparatorsOffset);
            }
        }
    }

    return myTableRow;
}

void LoggerTableProxyModel::createNewItemModel(bool bSetAsModel)
{
    QMutexLocker<QMutex> myScopedMutex(myMutex);

    if (myItemModel != nullptr) {
        myItemModel->deleteLater();
    }

    myItemModel = new LoggerTableItemModel(this, this);

    if (naColumnOrder.isEmpty() == true) {
        myItemModel->setHeaderData(0, Qt::Horizontal, "Log entry", Qt::DisplayRole);

    } else {
        myItemModel->setColumnCount(static_cast<int>(naColumnOrder.size())); //first column is the log number

        for (int nIndex = 0; nIndex < naColumnOrder.size(); ++nIndex) {
            myItemModel->setHeaderData(nIndex,
                                       Qt::Horizontal,
                                       getColumnName(static_cast<LoggerEnum::Columns>(naColumnOrder[nIndex])),
                                       Qt::DisplayRole);
        }
    }

    if (bSetAsModel == true) {
        setSourceModel(myItemModel);
        szaTableModelRaw.clear();
    }
}

void LoggerTableProxyModel::updateLoggerPatternCache()
{
    naLoggerPatternData.clear();
    naColumnOrder.clear();
    naColumnOrder.append(LoggerEnum::COLUMN_INDEX);

    bool bLastSegment = false;

    qsizetype nPatternIndex = 0;

    QChar cEndSeparator;
    QString szCurrentPattern;

    const qsizetype nPatternEnd = szLoggerPattern.size() - 1;

    while (bLastSegment == false) {

        patternData patternDataElement;

        int nDataStartOffset = 0;

        //get the next pattern
        while (nPatternIndex < nPatternEnd) {
            const QChar cPatternChar = szLoggerPattern[nPatternIndex];

            if (cPatternChar == '%') {
                szCurrentPattern = szLoggerPattern.mid(nPatternIndex, 2);

                LoggerEnum::LoggerPattern ePattern = static_cast<LoggerEnum::LoggerPattern>(szaLoggerPatternElements.indexOf(szCurrentPattern));

                //in case a pattern occupies more than 1 column (Class:Line could be separated)
                switch (ePattern) {
                    case LoggerEnum::PATTERN_TIMESTAMP:
                        naColumnOrder.append(LoggerEnum::COLUMN_TIMESTAMP);
                        break;

                    case LoggerEnum::PATTERN_CLASS:
                        naColumnOrder.append(LoggerEnum::COLUMN_CLASS);
                        break;

                    case LoggerEnum::PATTERN_SEVERITY:
                        naColumnOrder.append(LoggerEnum::COLUMN_SEVERITY);
                        break;

                    case LoggerEnum::PATTERN_MESSAGE:
                        naColumnOrder.append(LoggerEnum::COLUMN_MESSAGE);
                        break;

                    case LoggerEnum::PATTERN_THREADID:
                        naColumnOrder.append(LoggerEnum::COLUMN_THREADID);
                        break;

                    case LoggerEnum::COUNT_LOGGER_PATTERN:
                        break;

                }

                //TODO if pattern not valid, should continue looking
                patternDataElement.eCurrentPattern = ePattern;  //1

                ++nPatternIndex;
                break;

            } else { //skip other separator characters: [ | spaces, etc
                ++nDataStartOffset;
            }

            ++nPatternIndex;
        }

        patternDataElement.nDataStartOffset = nDataStartOffset;  //2

        //nPatternIndex is currently at the letter after the %

        //TODO confusing
        if ((nPatternIndex + 1) >= nPatternEnd) {
            bLastSegment = true;

        } else {
            if (szLoggerPattern[nPatternIndex + 1] == '{') {
                if ((nPatternIndex + 2) >= nPatternEnd) { //should not happen, unless there is a stray { at the end of the pattern string
                    cEndSeparator = '{';

                } else {
                    nPatternIndex = szLoggerPattern.indexOf('}', nPatternIndex + 2);

                    if (nPatternIndex != -1
                        && (nPatternIndex + 1) <= nPatternEnd) { //found {...}, so the end separator is the char after '}'

                        cEndSeparator = szLoggerPattern[nPatternIndex + 1];
                        ++nPatternIndex;

                    } else {
                        bLastSegment = true;
                    }
                }

            } else {
                cEndSeparator = szLoggerPattern[nPatternIndex + 1];
                ++nPatternIndex;
            }
        }

        if (bLastSegment == true) {
            patternDataElement.cEndSeparator = SEPARATOR_END;  //3

        } else {
            patternDataElement.cEndSeparator = cEndSeparator;  //3
        }

        naLoggerPatternData.append(patternDataElement);
        patternDataElement.print();
    }

}

//TODO may crash when the pattern doesn't match the data
void LoggerTableProxyModel::reparseTableData()
{
    createNewItemModel();

    nRowIndexCount = nRowIndexCount - this->rowCount(); //keep the index

    appendRows(szaTableModelRaw, false);

    setSourceModel(myItemModel);
}

qsizetype LoggerTableProxyModel::getEndIndexGeneric(const QString &szRowData, int nStartIndex, const QChar cEndSeparator, bool bIgnoreExtraSeparators, int &nExtraSeparatorsOffset)
{
    const qsizetype nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    if (bIgnoreExtraSeparators == true) {
        nExtraSeparatorsOffset = 0;

        qsizetype nEndIndexOffsetted = nEndIndex;

        const qsizetype nSize = szRowData.size();

        while (++nEndIndexOffsetted < nSize
               && szRowData[nEndIndexOffsetted] == cEndSeparator) {
            ++nExtraSeparatorsOffset;
        }
    }

    return nEndIndex;
}

qsizetype LoggerTableProxyModel::getEndIndexTimestamp(const QString &szRowData, int nStartIndex, const QChar cEndSeparator, bool bIgnoreExtraSeparators, int &nExtraSeparatorsOffset)
{
    //TODO deal with other timestamp patterns
    qsizetype nEndIndex;

    nEndIndex = szRowData.indexOf(' ', nStartIndex); // date and time separator
    nEndIndex = szRowData.indexOf(cEndSeparator, nEndIndex + 1);

    if (bIgnoreExtraSeparators == true) {
        nExtraSeparatorsOffset = 0;

        qsizetype nEndIndexOffsetted = nEndIndex;

        const qsizetype nSize = szRowData.size();

        while (++nEndIndexOffsetted < nSize
               && szRowData[nEndIndexOffsetted] == cEndSeparator) {
            ++nExtraSeparatorsOffset;
        }
    }

    return nEndIndex;
}

void LoggerTableProxyModel::clear()
{
    createNewItemModel(true);

//    setFilterRegExp(QRegExp());
}

void LoggerTableProxyModel::parseFile(const QString &szFilename)
{
    if (FileUtils::isFileTypeLog(szFilename) == false) {
        Q_EMIT fileParsingResult(GlobalConstants::ERROR);
        return;
    }

    QFile myFile(szFilename);

    if (myFile.open(QFile::ReadOnly) == true) {

        resetIndex();

        createNewItemModel();

        QTextStream myTextStream(&myFile);

        while (myTextStream.atEnd() == false) {
            const QString szRowData = myTextStream.readLine();

            appendRow(szRowData);
        }

        setSourceModel(myItemModel);

        Q_EMIT fileParsingResult(GlobalConstants::SUCCESS, szFilename);

    } else {
        Q_EMIT fileParsingResult(GlobalConstants::ERROR);
    }

}

void LoggerTableProxyModel::parseClipboard()
{
    resetIndex();

    createNewItemModel(); //this way is faster: create a new model, insert every row, and set as model in the end
    szaTableModelRaw.clear();

    QString szClipboardData = QApplication::clipboard()->text();

// #define DEBUG_SPEED
#ifdef DEBUG_SPEED
    TimeUtils::startTimer();
#endif

    QTextStream myTextStream(&szClipboardData); //faster than string.split with either text or regex

    while (myTextStream.atEnd() == false) {
        appendRow(myTextStream.readLine());
    }

#ifdef DEBUG_SPEED
    TimeUtils::printTimeMilliseconds();
#endif

    setSourceModel(myItemModel);

    if (myItemModel->rowCount() == 0) {
        Q_EMIT clipboardParsingResult(GlobalConstants::ERROR);

    } else {
        Q_EMIT clipboardParsingResult(GlobalConstants::SUCCESS);
    }
}

void LoggerTableProxyModel::newMessageReceived(const QString &szMessage)
{
    QMutexLocker<QMutex> myScopedMutex(myMutex);

    const QStringList szaMessages = szMessage.split('\n', Qt::SkipEmptyParts, Qt::CaseInsensitive);

    appendRows(szaMessages);
}

void LoggerTableProxyModel::setLoggerPattern(const QString &szPattern)
{
    this->szLoggerPattern = szPattern;

    updateLoggerPatternCache();
    reparseTableData();
}

void LoggerTableProxyModel::menuActionClickedFilter(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    const QString szActionData = mySenderAction->data().toString();
    const QStringList myActionDataSplit = szActionData.split('#');

    const QString &szFilter = myActionDataSplit.at(0);
    const int nColumn = static_cast<QString>(myActionDataSplit.at(1)).toInt();

    //TODO add the filter to a line edit so the user can change it
    setFilterRegExp(QRegularExpression("\\b" + szFilter + "\\b"));
    setFilterKeyColumn(nColumn);
}

void LoggerTableProxyModel::menuActionClickedOpenFile(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    const QString szActionData = mySenderAction->data().toString();
    const QStringList myActionDataSplit = szActionData.split('#');

    const QString &szClassName = myActionDataSplit.at(0);
    const QString &szLineNumber = myActionDataSplit.at(1);

    //TODO parse files on App init or source path changes, and inside SourceCodeHandler itself
    const QString szCurrentProjectName = SourceCodeHandler::getCurrentProjectName();
    const QStringList szaSourceLocations = SourceCodeHandler::getSourceCodeLocations();

    QStringList szaSourceFolders;

    for (const QString &szSourceLocation : szaSourceLocations) {
        const QStringList szaSourceLocation = szSourceLocation.split(GlobalConstants::SEPARATOR_SETTINGS_LIST_2);

        const QString &szProjectName = szaSourceLocation.at(SourceCodeLocationsEnum::COLUMN_PROJECT_NAME);

        if (szProjectName.isEmpty()
            || szProjectName == szCurrentProjectName) { //empty name means this path applies to all projects

            szaSourceFolders.append(szaSourceLocation.at(SourceCodeLocationsEnum::COLUMN_SOURCE_PATH));
        }
    }

    const QString szFileExtension = SourceCodeHandler::getCurrentLanguageFileExtension();

    if (szFileExtension.isEmpty()) {
        Q_EMIT showNotification(tr("Current language not configured"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    QString szFilenameFullPath;

    bool bFileFound = false;

    for (int nIndex = 0; nIndex < szaSourceFolders.size() && bFileFound == false; ++nIndex) {
        QDirIterator myDirIterator(szaSourceFolders[nIndex], QStringList() << '*' + szFileExtension, QDir::Files, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

        while (myDirIterator.hasNext()) {

            const QString szCurrent = myDirIterator.next();
//            qDebug() << szCurrent;

            if (szCurrent.endsWith('/' + szClassName + szFileExtension) == true) {
                szFilenameFullPath = szCurrent;
                bFileFound = true;
                break;
            }
        }
    }

    if (bFileFound == false) {
        Q_EMIT showNotification(tr("No source file found"), ToastNotificationWidget::ERROR, 2000);

    } else {
        GlobalConstants::ErrorCode eResult = SourceCodeHandler::openFileInEditor(SourceCodeHandler::getCurrentEditor(), szFilenameFullPath, szLineNumber);

        if (eResult == GlobalConstants::SUCCESS) {
            Q_EMIT showNotification(tr("Opened file\n") + szFilenameFullPath + tr("\nin editor ") + SourceCodeHandler::getCurrentEditorName(), ToastNotificationWidget::SUCCESS, 2000);

        } else {
            Q_EMIT showNotification(tr("Error opening file\n") + szFilenameFullPath + tr("\nin editor ") + SourceCodeHandler::getCurrentEditorName(), ToastNotificationWidget::ERROR, 3000);
        }
    }
}

void LoggerTableProxyModel::menuActionClickedCopySelected(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    const int nColumnIndex = mySenderAction->data().toInt();

    Q_EMIT copySelectedData(nColumnIndex);
}

void LoggerTableProxyModel::deleteRowsAbove(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    const int nLineNumber = mySenderAction->data().toInt();

    myItemModel->removeRows(0, nLineNumber);
}

void LoggerTableProxyModel::deleteRowsBelow(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    const int nLineNumber = mySenderAction->data().toInt();

    myItemModel->removeRows(nLineNumber + 1, myItemModel->rowCount() - nLineNumber - 1);
}

void LoggerTableProxyModel::updateKeywords(const QStringList &szaKeywords)
{
    myItemModel->updateKeywords(szaKeywords);
}

