#include "LoggerTableProxyModel.h"
#include "application/GlobalConstants.h"
#include "application/SourceCodeHandler.h"
#include "data/LoggerTableItemModel.h"
#include "ui/LoggerClientWidget.h"
#include "ui/ToastNotificationWidget.h"
#include "util/FileUtils.h"
#include "util/MemoryUtils.h"
#include "util/TimeUtils.h"

QVector<QString> LoggerTableProxyModel::szaLoggerPatternElements;
QVector<QString> LoggerTableProxyModel::szaLoggerSeverityNames;

LoggerTableProxyModel::LoggerTableProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , myItemModel(nullptr)
    , myMutex(new QMutex(QMutex::NonRecursive))
{
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

    if (szLoggerPattern.isEmpty() == true) {
        myItemModel->appendRow(new QStandardItem(szRowDataTrim));

    } else {
        QList<QStandardItem *> myTableRow = parseLogMessage(szRowData);

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
        this->appendRow(szRowData, bAppendToRawData);
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

void LoggerTableProxyModel::setFilterRegExp(const QRegExp &regExp)
{
    if (regExp.isValid() == true) {
        emit filterStateChanged(true);

    } else {
        emit filterStateChanged(false);
    }

    QSortFilterProxyModel::setFilterRegExp(regExp);
}

void LoggerTableProxyModel::setFilterRegExp(const QString &szPattern)
{
    if (szPattern.size() != 0) {
        emit filterStateChanged(true);

    } else {
        emit filterStateChanged(false);
    }

    QSortFilterProxyModel::setFilterRegExp(szPattern);
}

int LoggerTableProxyModel::getVisibleIndexForColumn(const LoggerEnum::Columns eColumn)
{
    return naColumnOrder.indexOf(eColumn);
}

LoggerEnum::Columns LoggerTableProxyModel::getColumnForVisibleIndex(const int nIndex)
{
    return static_cast<LoggerEnum::Columns>(naColumnOrder.at(nIndex));
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
    if (szaLoggerPatternElements.size() != 0) {
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
    if (szaLoggerSeverityNames.size() != 0) {
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
/// Ex: timestamp always starts at posX and ends with ' ', classname always start at posZ and ends with ']' and so on
/// These known elements can be parsed when the pattern changes, put in two vectors and just iterate the vector and do the switch case part for each row
QList<QStandardItem *> LoggerTableProxyModel::parseLogMessage(const QString &szRowData)
{
    QList<QStandardItem *> myTableRow;
    myTableRow.reserve(myItemModel->columnCount() + 1);

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

        myTableRow.append(new QStandardItem(QString::number(myItemModel->rowCount() + 1) + "*"));

        for (int nIndex = 1; nIndex < LoggerEnum::COLUMN_MESSAGE; ++nIndex) {
            QStandardItem *myPreviousItem = myItemModel->item(nPreviousRow, nIndex);

            if (myPreviousItem != nullptr) {
                QStandardItem *myPreviousItemClone = myPreviousItem->clone();
                myTableRow.append(myPreviousItemClone);
            }
        }

        myTableRow.append(new QStandardItem(szRowData));
        return myTableRow;

    } else {
        myTableRow.append(new QStandardItem(QString::number(myItemModel->rowCount() + 1)));

#define PARSE_MESSAGE_CORRECTLY
#ifdef PARSE_MESSAGE_CORRECTLY

        int nDataStartIndex = 0;
        int nDataEndIndex = 0;

        for (int nIndex = 0; nIndex < naLoggerPatternData.size(); nIndex += 3) {

            LoggerEnum::LoggerPattern nCurrentPattern = static_cast<LoggerEnum::LoggerPattern>(naLoggerPatternData.at(nIndex));
            int nStartOffset = naLoggerPatternData.at(nIndex + 1);

            if (naLoggerPatternData.at(nIndex + 2) == -1) { //last element
                myTableRow.append(new QStandardItem(szRowData.midRef(nDataStartIndex + nStartOffset).trimmed().toString()));

            } else {

                QChar cEndSeparator = QChar::fromLatin1(static_cast<char>(naLoggerPatternData.at(nIndex + 2)));
//                cEndSeparator = ' ';
                nDataStartIndex += nStartOffset;

//                while (szRowData.at(nDataStartIndex).isSpace()) { //NOTE only needed when parsing clipboard or file, because of extra spaces between "columns"
//                    ++nDataStartIndex;
//                }

                switch (nCurrentPattern) {
                    case LoggerEnum::PATTERN_TIMESTAMP: {
                        //TODO place date and time in different columns
                        nDataEndIndex = getEndIndexTimestamp(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.midRef(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed().toString()));
                        break;
                    }

                    case LoggerEnum::PATTERN_THREADID: {
                        nDataEndIndex = getEndIndexThreadId(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.midRef(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed().toString()));
                        break;
                    }

                    case LoggerEnum::PATTERN_SEVERITY: {
                        nDataEndIndex = getEndIndexSeverity(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.midRef(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed().toString()));
                        break;
                    }

                    case LoggerEnum::PATTERN_CLASS: {
                        //TODO place line number in another column
                        nDataEndIndex = getEndIndexClass(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.midRef(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed().toString()));
                        break;
                    }

                    case LoggerEnum::PATTERN_MESSAGE: {
                        nDataEndIndex = getEndIndexMessage(szRowData, nDataStartIndex, cEndSeparator);
                        myTableRow.append(new QStandardItem(szRowData.midRef(nDataStartIndex, nDataEndIndex - nDataStartIndex).trimmed().toString()));
                        break;
                    }

                    case LoggerEnum::COUNT_LOGGER_PATTERN:
                    default:
                        break;
                }
            }

            nDataStartIndex = nDataEndIndex + 1;
        }

#else
        int nIndexStart = 0;
        int nIndexEnd = 0;
        nIndexStart = szRowData.indexOf(" ", 0);
        nIndexStart = szRowData.indexOf(" ", ++nIndexStart);
        QString szTimeStamp = szRowData.left(nIndexStart);

        nIndexEnd = szRowData.indexOf(" ", ++nIndexStart);
        QString szThreadId = szRowData.mid(nIndexStart, nIndexEnd - nIndexStart);
        nIndexStart = nIndexEnd;

        nIndexEnd = szRowData.indexOf("]", ++nIndexStart);
        QString szClassName = szRowData.mid(nIndexStart + 1, nIndexEnd - nIndexStart - 1);
        nIndexStart = nIndexEnd;

        ++nIndexStart; //last one was ], after it is the space, so increment again here
        nIndexEnd = szRowData.indexOf(" ", ++nIndexStart);
        QString szSeverity = szRowData.mid(nIndexStart, nIndexEnd - nIndexStart);
        nIndexStart = nIndexEnd;

        nIndexStart = szRowData.indexOf(" ", ++nIndexStart);
        QString szMessage = szRowData.mid(nIndexStart);

        myTableRow.append(new QStandardItem(szTimeStamp.trimmed()));
        myTableRow.append(new QStandardItem(szThreadId.trimmed()));
        myTableRow.append(new QStandardItem(szClassName.trimmed()));
        myTableRow.append(new QStandardItem(szSeverity.trimmed()));
        myTableRow.append(new QStandardItem(szMessage.trimmed()));

#endif
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
    ///this vector is composed of groups of 3 ints
    /// 1-current pattern
    /// 2-offset for start index (to account for extra separators between elements)
    /// 3-char of the end separator for the pattern in 1. For the last group, this char is -1 (to the end of the message)

    naLoggerPatternData.clear();
    naColumnOrder.clear();
    naColumnOrder.append(LoggerEnum::COLUMN_INDEX);

    bool bLastSegment = false;

    int nPatternIndex = 0;

    QChar cEndSeparator;
    QString szCurrentPattern;

    int nPatternEnd = szLoggerPattern.size() - 1;

    while (bLastSegment == false) {

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

                naLoggerPatternData.append(ePattern);     //1

                ++nPatternIndex;
                break;

            } else if (cPatternChar.isSpace() == false) { //skip other separator characters: [ | etc
                ++nDataStartOffset;
            }

            ++nPatternIndex;
        }

        naLoggerPatternData.append(nDataStartOffset);   //2

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
            naLoggerPatternData.append(-1);    //3

        } else {
            naLoggerPatternData.append(static_cast<int>(cEndSeparator.toLatin1()));    //3
        }
    }

}

void LoggerTableProxyModel::reparseTableData()
{
    createNewItemModel();

    appendRows(szaTableModelRaw, false);

    this->setSourceModel(myItemModel);
}

int LoggerTableProxyModel::getEndIndexClass(const QString &szRowData, int nStartIndex, QChar cEndSeparator)
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexMessage(const QString &szRowData, int nStartIndex, QChar cEndSeparator)
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexSeverity(const QString &szRowData, int nStartIndex, QChar cEndSeparator)
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexThreadId(const QString &szRowData, int nStartIndex, QChar cEndSeparator)
{
    int nEndIndex = szRowData.indexOf(cEndSeparator, nStartIndex);

    return nEndIndex;
}

int LoggerTableProxyModel::getEndIndexTimestamp(const QString &szRowData, int nStartIndex, QChar cEndSeparator)
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

    setFilterRegExp(QRegExp());
}

void LoggerTableProxyModel::parseFile(const QString &szFilename)
{
    if (FileUtils::isFileTypeLog(szFilename) == false) {
        emit fileParsingResult(false);
        return;
    }

    QFile myFile(szFilename);

    if (myFile.open(QFile::ReadOnly) == true) {

        createNewItemModel();

        QTextStream myTextStream(&myFile);

        while (myTextStream.atEnd() == false) {
            QString szRowData = myTextStream.readLine();

            this->appendRow(szRowData);
        }

        this->setSourceModel(myItemModel);

        emit fileParsingResult(true, QFileInfo(szFilename).fileName());

    } else {
        emit fileParsingResult(false);
    }

}

void LoggerTableProxyModel::parseClipboard()
{
    TimeUtils::startTimer();

    createNewItemModel(); //this way is faster: create a new model, insert every row, and set as model in the end

    QString szClipboardData = QApplication::clipboard()->text();

    QTextStream myTextStream(&szClipboardData);

    while (myTextStream.atEnd() == false) {
        QString szRowData = myTextStream.readLine();

        this->appendRow(szRowData);
    }

    this->setSourceModel(myItemModel);

    TimeUtils::printTimeMilliseconds();

    if (myItemModel->rowCount() == 0) {
        emit clipboardParsingResult(false);

    } else {
        emit clipboardParsingResult(true);
    }
}

void LoggerTableProxyModel::newMessageReceived(const QString &szMessage)
{
    QMutexLocker myScopedMutex(myMutex);

    QVector<QStringRef> szraMessages = szMessage.splitRef('\n');

    for (const QStringRef &szrMessage : szraMessages) {
        if (szrMessage.trimmed().isEmpty() == false) {
            this->appendRow(szrMessage.toString());
        }
    }
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

    emit filterStateChanged(true);
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

