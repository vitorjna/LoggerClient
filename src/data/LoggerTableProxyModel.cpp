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

void LoggerTableProxyModel::appendRow(const QString &szRowData, bool bAppendToRawData)
{
    const QString szRowDataTrim = szRowData.trimmed();

    if (szLoggerPattern.isEmpty() == true) {
        myItemModel->appendRow(new QStandardItem(szRowDataTrim));

        if (bAppendToRawData) {
            szaTableModelRaw.append(szRowDataTrim);
        }

    } else {
        QList<QStandardItem *> myTableRow = parseLogMessage(szRowDataTrim);

        if (myTableRow.isEmpty() == false) {
            myItemModel->appendRow(myTableRow);

            if (bAppendToRawData) {
                szaTableModelRaw.append(szRowDataTrim);
            }
        }
    }
}

void LoggerTableProxyModel::appendRows(const QStringList &szaRowsData, bool bAppendToRawData)
{
    for (const QString &szRowData : szaRowsData) {
        this->appendRow(szRowData, bAppendToRawData);
    }
}

QColor LoggerTableProxyModel::calculateColorForLevel(const QString &szLevel)
{
//    static int nLevels = {0, 32, 64, 96, 128, 160, 192, 224, 255};

    switch (static_cast<LoggerEnum::LoggerSeverity>(szaLoggerSeverityNames.indexOf(szLevel))) {
        case LoggerEnum::TRACE:
            return QColor(0, 64, 64, 192);

        case LoggerEnum::DEBUG_L:
            return QColor(0, 96, 96, 255);

        case LoggerEnum::INFO:
            return QColor(128, 128, 0, 192);

        case LoggerEnum::WARNING:
            return QColor(128, 128, 0, 255);

        case LoggerEnum::ERROR_L:
            return QColor(128, 0, 0, 192);

        case LoggerEnum::FATAL:
            return QColor(128, 0, 0, 255);

        case LoggerEnum::NONE:
        case LoggerEnum::COUNT_LOGGER_SEVERITY:
            return QColor(Qt::gray);
    }
}

QList<QAction *> *LoggerTableProxyModel::generateActionsForIndex(const QModelIndex &myModelIndex, QWidget *parent)
{
    if (myModelIndex.isValid() == false) {
        return nullptr;
    }

    QList<QAction *> *myActionList = new QList<QAction *>();

    switch (static_cast<LoggerEnum::Columns>(myModelIndex.column())) {
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

QString LoggerTableProxyModel::getColumnName(LoggerEnum::Columns column)
{
    switch (column) {
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
        if (szRowData.contains(QRegExp("\\b" + szaLoggerSeverityNames.at(nIndex) + "\\b", Qt::CaseSensitive)) == true) {
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
//            if (nIndex == LoggerEnum::COLUMN_TIMESTAMP
//                || nIndex == LoggerEnum::COLUMN_SEVERITY
//                || nIndex == LoggerEnum::COLUMN_THREADID) {

//                tableRow->append(new QStandardItem(QLatin1String("")));
//                continue;
//            }

            QStandardItem *myPreviousItem = myItemModel->item(nPreviousRow, nIndex);

            if (myPreviousItem != nullptr) {
                QStandardItem *myPreviousItemClone = myPreviousItem->clone();
                myTableRow.append(myPreviousItemClone);
            }
        }

        myTableRow.append(new QStandardItem(szRowData.trimmed()));
        return myTableRow;

    } else {
        myTableRow.append(new QStandardItem(QString::number(myItemModel->rowCount() + 1)));

#define PARSE_MESSAGE_CORRECTLY
#ifdef PARSE_MESSAGE_CORRECTLY

        int nDataStartIndex = 0;
        int nDataEndIndex = 0;

        for (int nIndex = 0; nIndex < naLoggerPatternData.size(); nIndex += 3) {

            int nCurrentPattern = naLoggerPatternData.at(nIndex);
            int nStartOffset = naLoggerPatternData.at(nIndex + 1);

            if (naLoggerPatternData.at(nIndex + 2) == -1) { //last element
                myTableRow.append(new QStandardItem(szRowData.mid(nDataStartIndex + nStartOffset).trimmed()));

            } else {

                QChar cEndSeparator = QChar::fromLatin1(static_cast<char>(naLoggerPatternData.at(nIndex + 2)));
//                cEndSeparator = ' ';
                nDataStartIndex += nStartOffset;

                while (szRowData.at(nDataStartIndex).isSpace()) { //NOTE only needed when parsing clipboard or file, because of extra spaces between "columns"
                    ++nDataStartIndex;
                }

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
    if (myItemModel != nullptr) {
        myItemModel->deleteLater();
    }

    myItemModel = new LoggerTableItemModel(this);

    if (szLoggerPattern.isEmpty() == true) {
        myItemModel->setHeaderData(0, Qt::Horizontal, "Log entry", Qt::DisplayRole);

    } else {
        myItemModel->setColumnCount(LoggerEnum::COUNT_TABLE_COLUMNS); //first column is the log number

        //TODO use this "for" in the currently enabled columns (method to be implemented)
        for (int nIndex = 0; nIndex < LoggerEnum::COUNT_TABLE_COLUMNS; ++nIndex) {
            myItemModel->setHeaderData(nIndex,
                                       Qt::Horizontal,
                                       getColumnName(static_cast<LoggerEnum::Columns>(nIndex)),
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
                naLoggerPatternData.append(szaLoggerPatternElements.indexOf(szCurrentPattern));     //1
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
    QMutexLocker myScopedMutex(myMutex);

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
    createNewItemModel(); //this way is faster: create a new model, insert every row, and set as model in the end

    QString szClipboardData = QApplication::clipboard()->text();

    QTextStream myTextStream(&szClipboardData);

    while (myTextStream.atEnd() == false) {
        QString szRowData = myTextStream.readLine();

        this->appendRow(szRowData);
    }

    this->setSourceModel(myItemModel);

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
            qDebug() << szCurrent;

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
        SourceCodeHandler::openFileInEditor(SourceCodeHandler::getCurrentEditor(), szFilenameFullPath, szLineNumber);

        emit showNotification(tr("File opened: ") + szFilenameFullPath, ToastNotificationWidget::SUCCESS, 2000);
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

