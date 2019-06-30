#include "LoggerTableProxyModel.h"
#include "application/SourceCodeHandler.h"
#include "data/LoggerTableItemModel.h"
#include "ui/LoggerClientWidget.h"
#include "ui/ToastNotificationWidget.h"
#include "util/FileUtils.h"
#include "util/MemoryUtils.h"
#include "util/TimeUtils.h"

QVector<QString> LoggerTableProxyModel::szaLoggerPatternElements;
QVector<QString> LoggerTableProxyModel::szaLoggerSeverityNames;
const QChar      LoggerTableProxyModel::END_SEPARATOR{'\0'};

LoggerTableProxyModel::LoggerTableProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , myItemModel(nullptr)
    , nRowIndexCount(0)
    , myMutex(new QMutex(QMutex::NonRecursive))
{
    szaTableModelRaw.reserve(10000);

    fillLoggerPatternElements();
    fillLoggerSeverityNames();

    createNewItemModel(true);
}

LoggerTableProxyModel::~LoggerTableProxyModel()
{
    MemoryUtils::deleteMutex(myMutex);
}

int LoggerTableProxyModel::getLogSeverityFromName(const QString &szSeverity)
{
    return szaLoggerSeverityNames.indexOf(szSeverity);
}

void LoggerTableProxyModel::appendRow(const QString &szRowData, bool bAppendToRawData)
{
    const QString szRowDataTrim = szRowData.trimmed();

    if (szRowDataTrim.isEmpty() == true) {
        return;
    }

    if (szLoggerPattern.isEmpty() == true) {
        myItemModel->appendRow(new QStandardItem(szRowDataTrim));

    } else {
        const QList<QStandardItem *> myTableRow = parseLogMessage(szRowDataTrim);

        if (myTableRow.isEmpty() == false) {
            myItemModel->appendRow(myTableRow);
        }
    }

    if (bAppendToRawData) {
        szaTableModelRaw.append(szRowDataTrim);
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
            QString szThreadId = myModelIndex.data(Qt::DisplayRole).toString();

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

            int nIndexOfLineNumber = szClassName.indexOf(':');

            if (nIndexOfLineNumber != -1) {
                QString szLineNumber = szClassName.mid(nIndexOfLineNumber + 1);

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
            QString szLogLevel = myModelIndex.data(Qt::DisplayRole).toString();

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

        case LoggerEnum::COLUMN_MESSAGE:
            break;

        case LoggerEnum::COUNT_TABLE_COLUMNS:
            break;
    }

    {
        int nLineNumber = myModelIndex.row();

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

void LoggerTableProxyModel::setFilterRegExp(const QRegExp &regExp, bool bActualFilter)
{
    if (regExp.isValid() == true) {
        myFilter = regExp;
        QSortFilterProxyModel::setFilterRegExp(regExp);

        emit filterStateChanged(bActualFilter); //if search was text, always emit false (not actual filter, just text)

    } else {
        myFilter = QRegExp();
        QSortFilterProxyModel::setFilterRegExp(regExp);

        emit filterStateChanged(false);
    }
}

void LoggerTableProxyModel::reApplyFilter()
{
    if (myFilter.pattern().isEmpty() == false) {
        setFilterRegExp(myFilter);
    }
}

int LoggerTableProxyModel::getVisibleIndexForColumn(const LoggerEnum::Columns eColumn)
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
            return QLatin1String("");
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
    szaLoggerSeverityNames[LoggerEnum::ERROR_L]            = QStringLiteral("ERROR");
    szaLoggerSeverityNames[LoggerEnum::FATAL]              = QStringLiteral("FATAL");
    szaLoggerSeverityNames[LoggerEnum::NONE]               = QStringLiteral("NONE");
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

    for (int nIndex = 0; nIndex < LoggerEnum::COUNT_LOGGER_SEVERITY; ++nIndex) {
        //replaced contains of Regex (\b + Level + \b) with just QString. May have false positives, but performance: 63 to 34ms, for the same workload
        if (szRowData.contains(szaLoggerSeverityNames.at(nIndex), Qt::CaseSensitive) == true) {
            bContinueFromLast = false; //one of the levels exists, create new entry
            break;
        }
    }

    if (bContinueFromLast == true) {
        int nPreviousRow = myItemModel->rowCount() - 1;

        if (nPreviousRow < 0) {
            myTableRow.clear();
            return myTableRow;
        }

        myTableRow.append(new QStandardItem(QString::number(++nRowIndexCount) + "*"));

        //copy all columns except message
        for (int nIndex = 1; nIndex < LoggerEnum::COLUMN_MESSAGE; ++nIndex) {
            QStandardItem *myPreviousItem = myItemModel->item(nPreviousRow, nIndex);

            if (myPreviousItem != nullptr) {
                QStandardItem *myPreviousItemClone = myPreviousItem->clone();
                myTableRow.append(myPreviousItemClone);
            }
        }

        myTableRow.append(new QStandardItem(szRowData));

    } else {
        myTableRow.append(new QStandardItem(QString::number(++nRowIndexCount)));

        int nDataStartIndex = 0;

        for (const patternData &patternDataElement : qAsConst(naLoggerPatternData)) {

            const LoggerEnum::LoggerPattern nCurrentPattern = patternDataElement.eCurrentPattern;
            const int nStartOffset                          = patternDataElement.nDataStartOffset;
            const QChar cEndSeparator                       = patternDataElement.cEndSeparator;

            if (cEndSeparator == END_SEPARATOR) { //last element
                myTableRow.append(new QStandardItem(szRowData.midRef(nDataStartIndex + nStartOffset).trimmed().toString()));

            } else {
//                cEndSeparator = ' ';
                nDataStartIndex += nStartOffset;

//                while (szRowData.at(nDataStartIndex).isSpace()) { //NOTE only needed when parsing clipboard or file, because of extra spaces between "columns"
//                    ++nDataStartIndex;
//                }

                int nDataEndIndex;

                switch (nCurrentPattern) {
                    case LoggerEnum::PATTERN_TIMESTAMP: {
                        //TODO place date and time in different columns
                        nDataEndIndex = getEndIndexTimestamp(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed()));
                        break;
                    }

                    case LoggerEnum::PATTERN_THREADID: {
                        nDataEndIndex = getEndIndexThreadId(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed()));
                        break;
                    }

                    case LoggerEnum::PATTERN_SEVERITY: {
                        nDataEndIndex = getEndIndexSeverity(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed()));
                        break;
                    }

                    case LoggerEnum::PATTERN_CLASS: {
                        //TODO place line number in another column
                        nDataEndIndex = getEndIndexClass(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed()));
                        break;
                    }

                    case LoggerEnum::PATTERN_MESSAGE: {
                        nDataEndIndex = getEndIndexMessage(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed()));
                        break;
                    }

                    case LoggerEnum::COUNT_LOGGER_PATTERN:
                    default:
                        nDataEndIndex = 0;
                        break;
                }

                nDataStartIndex = nDataEndIndex + 1;
            }

        }
    }

    return myTableRow;
}

void LoggerTableProxyModel::createNewItemModel(bool bSetAsModel)
{
    QMutexLocker myScopedMutex(myMutex);

    if (myItemModel != nullptr) {
        myItemModel->deleteLater();
    }

    myItemModel = new LoggerTableItemModel(this, this);

    if (naColumnOrder.isEmpty() == true) {
        myItemModel->setHeaderData(0, Qt::Horizontal, "Log entry", Qt::DisplayRole);

    } else {
        myItemModel->setColumnCount(naColumnOrder.size()); //first column is the log number

        for (int nIndex = 0; nIndex < naColumnOrder.size(); ++nIndex) {
            myItemModel->setHeaderData(nIndex,
                                       Qt::Horizontal,
                                       getColumnName(static_cast<LoggerEnum::Columns>(naColumnOrder.at(nIndex))),
                                       Qt::DisplayRole);
        }
    }

    if (bSetAsModel == true) {
        this->setSourceModel(myItemModel);
        szaTableModelRaw.clear();
    }
}

void LoggerTableProxyModel::updateLoggerPatternCache()
{
    naLoggerPatternData.clear();
    naColumnOrder.clear();
    naColumnOrder.append(LoggerEnum::COLUMN_INDEX);

    bool bLastSegment = false;

    int nPatternIndex = 0;

    QChar cEndSeparator;
    QString szCurrentPattern;

    int nPatternEnd = szLoggerPattern.size() - 1;

    while (bLastSegment == false) {

        patternData patternDataElement;

        int nDataStartOffset = 0;

        //get the next pattern
        while (nPatternIndex < nPatternEnd) {
            QCharRef cPatternChar = szLoggerPattern[nPatternIndex];

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
                    default:
                        break;

                }

                //TODO if pattern not valid, should continue looking
                patternDataElement.eCurrentPattern = ePattern;  //1

                ++nPatternIndex;
                break;

            } else if (cPatternChar.isSpace() == false) { //skip other separator characters: [ | etc
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
            patternDataElement.cEndSeparator = END_SEPARATOR;  //3

        } else {
            patternDataElement.cEndSeparator = cEndSeparator;  //3
        }

        naLoggerPatternData.append(patternDataElement);
    }

}

void LoggerTableProxyModel::reparseTableData()
{
    createNewItemModel();

    appendRows(szaTableModelRaw, false);

    this->setSourceModel(myItemModel);
}

int LoggerTableProxyModel::getEndIndexClass(const QString &szRowData, int nStartIndex, QChar cEndSeparator) const
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexMessage(const QString &szRowData, int nStartIndex, QChar cEndSeparator) const
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexSeverity(const QString &szRowData, int nStartIndex, QChar cEndSeparator) const
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexThreadId(const QString &szRowData, int nStartIndex, QChar cEndSeparator) const
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexTimestamp(const QString &szRowData, int nStartIndex, QChar cEndSeparator) const
{
    //TODO deal with other timestamp patterns
    int nEndIndex = -1;

    nEndIndex = szRowData.indexOf(' ', nStartIndex); // date and time separator
    nEndIndex = szRowData.indexOf(cEndSeparator, nEndIndex + 1);

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
        emit fileParsingResult(GlobalConstants::ERROR);
        return;
    }

    QFile myFile(szFilename);

    if (myFile.open(QFile::ReadOnly) == true) {

        resetIndex();

        createNewItemModel();

        QTextStream myTextStream(&myFile);

        while (myTextStream.atEnd() == false) {
            QString szRowData = myTextStream.readLine();

            this->appendRow(szRowData);
        }

        this->setSourceModel(myItemModel);

        emit fileParsingResult(GlobalConstants::SUCCESS, QFileInfo(szFilename).fileName());

    } else {
        emit fileParsingResult(GlobalConstants::ERROR);
    }

}

void LoggerTableProxyModel::parseClipboard()
{
//    TimeUtils::startTimer();

    resetIndex();

    createNewItemModel(); //this way is faster: create a new model, insert every row, and set as model in the end
    szaTableModelRaw.clear();

//#define DEBUG_SPEED
#ifdef DEBUG_SPEED
    this->setSourceModel(myItemModel);
#endif

    QString szClipboardData = QApplication::clipboard()->text();

    QTextStream myTextStream(&szClipboardData); //faster than string.split with either text or regex

    while (myTextStream.atEnd() == false) {
        const QString szRowData = myTextStream.readLine();

        this->appendRow(szRowData);
    }

#ifndef DEBUG_SPEED
    this->setSourceModel(myItemModel);
#endif

//    TimeUtils::printTimeMilliseconds();

    if (myItemModel->rowCount() == 0) {
        emit clipboardParsingResult(GlobalConstants::ERROR);

    } else {
        emit clipboardParsingResult(GlobalConstants::SUCCESS);
    }
}

void LoggerTableProxyModel::newMessageReceived(const QString &szMessage)
{
    QMutexLocker myScopedMutex(myMutex);

    const QStringList szaMessages = szMessage.split('\n', QString::SkipEmptyParts, Qt::CaseInsensitive);

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

    QString szActionData = mySenderAction->data().toString();
    QStringList myActionDataSplit = szActionData.split('#');

    const QString &szFilter = myActionDataSplit.at(0);
    int nColumn = static_cast<QString>(myActionDataSplit.at(1)).toInt();

    //TODO add the filter to a line edit so the user can change it
    this->setFilterRegExp(QRegExp("\\b" + szFilter + "\\b", Qt::CaseSensitive, QRegExp::RegExp));
    this->setFilterKeyColumn(nColumn);
}

void LoggerTableProxyModel::menuActionClickedOpenFile(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    QString szActionData = mySenderAction->data().toString();
    QStringList myActionDataSplit = szActionData.split('#');

    const QString &szClassName = myActionDataSplit.at(0);
    const QString &szLineNumber = myActionDataSplit.at(1);

    //TODO parse files on App init
    QStringList szaSourceFolders = SourceCodeHandler::getSourceCodeLocations();
    QString szFileExtension = SourceCodeHandler::getCurrentLanguageFileExtension();

    if (szFileExtension.isEmpty()) {
        emit showNotification(tr("Current language not configured"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    QString szFilenameFullPath;

    bool bFileFound = false;

    for (int nIndex = 0; nIndex < szaSourceFolders.size() && bFileFound == false; ++nIndex) {
        QDirIterator myDirIterator(szaSourceFolders.at(nIndex), QStringList() << '*' + szFileExtension, QDir::Files, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

        while (myDirIterator.hasNext()) {

            QString szCurrent = myDirIterator.next();
//            qDebug() << szCurrent;

            if (szCurrent.endsWith('/' + szClassName + szFileExtension) == true) {
                szFilenameFullPath = szCurrent;
                bFileFound = true;
                break;
            }
        }
    }

    if (bFileFound == false) {
        emit showNotification(tr("No source file found"), ToastNotificationWidget::ERROR, 2000);

    } else {
        GlobalConstants::ErrorCode eResult = SourceCodeHandler::openFileInEditor(SourceCodeHandler::getCurrentEditor(), szFilenameFullPath, szLineNumber);

        if (eResult == GlobalConstants::SUCCESS) {
            emit showNotification(tr("Opened file\n") + szFilenameFullPath + tr("\nin editor ") + SourceCodeHandler::getCurrentEditorName(), ToastNotificationWidget::SUCCESS, 2000);

        } else {
            emit showNotification(tr("Error opening file\n") + szFilenameFullPath + tr("\nin editor ") + SourceCodeHandler::getCurrentEditorName(), ToastNotificationWidget::ERROR, 3000);
        }
    }
}

void LoggerTableProxyModel::deleteRowsAbove(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    int nLineNumber = mySenderAction->data().toInt();

    myItemModel->removeRows(0, nLineNumber);
}

void LoggerTableProxyModel::deleteRowsBelow(bool bState)
{
    Q_UNUSED(bState)

    QAction *mySenderAction = qobject_cast<QAction *>(sender());

    if (mySenderAction == nullptr) {
        return;
    }

    int nLineNumber = mySenderAction->data().toInt();

    myItemModel->removeRows(nLineNumber + 1, myItemModel->rowCount() - nLineNumber - 1);
}

