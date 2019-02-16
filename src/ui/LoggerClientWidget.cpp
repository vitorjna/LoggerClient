#include "LoggerClientWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "data/LoggerTableProxyModel.h"
#include "network/ChannelSocketClient.h"
#include "ui/NetworkConnectionWidget.h"
#include "ui/OptionsWidget.h"
#include "ui/PushButtonWithMenu.h"
#include "ui/SearchWidget.h"
#include "ui/ToastNotificationWidget.h"
#include "util/FileUtils.h"
#include "util/MemoryUtils.h"
#include "util/TimeUtils.h"
#include "view/LoggerTreeView.h"

//TODO Filter by one column. Then filter by other column, Clear Table button becomes disabled
//TODO add option to color by thread
//TODO bug when parsing clipboard or file, end separator is always ' ', and not '[' as it is in the pattern

LoggerClientWidget::LoggerClientWidget(QWidget *parent)
    : QWidget(parent)
    , bUsingCustomColumnWidth(false)
    , eCurrentMode(COUNT_LOG_MODE)
    , bOpenFileAfterSavingPending(false)
    , bIsAtBottom(true)
    , myMutex(new QMutex(QMutex::NonRecursive))
    , myOptionsWidget(new OptionsWidget(this))
{
#ifdef DEBUG_STUFF
    initDebugStuff();
#endif

    this->hide();

    myChannelSocketClient = new ChannelSocketClient();

    setupUI();
    setStyleSheet(QLatin1String(""));
    setupSignalsAndSlots();
    setupShortcuts();
    loadSettings();

    setLogWidgetMode(EMPTY);

    this->show();
}

LoggerClientWidget::~LoggerClientWidget()
{
    MemoryUtils::deletePointer(myProxyModel);
    MemoryUtils::deleteMutex(myMutex);

    myWorkerThread.quit();
    myWorkerThread.wait(200);
}

void LoggerClientWidget::setStyleSheet(const QString &szStyleSheet)
{
    QWidget::setStyleSheet(szStyleSheet);
}

void LoggerClientWidget::saveWindowPosition()
{
    QPoint myPos = this->pos();

    QSize mySize = this->size();

    AppSettings::setValue(AppSettings::KEY_WINDOW_POS_MAIN,
                          QString::number(myPos.x())
                          + GlobalConstants::SETTINGS_STRING_SEPARATOR + QString::number(myPos.y())
                          + GlobalConstants::SETTINGS_STRING_SEPARATOR + QString::number(mySize.width())
                          + GlobalConstants::SETTINGS_STRING_SEPARATOR + QString::number(mySize.height())
                         );
}

void LoggerClientWidget::setupUI()
{
//    delete this->layout();

    //main window layout
    QVBoxLayout *myMainLayout = new QVBoxLayout(this);
    {
        //server connection / file opening
        QHBoxLayout *myTopLayout = new QHBoxLayout();
        {
            //server connection widget, can be disabled as a whole
            myServerConnectionWidget = new NetworkConnectionWidget(this);

            myTopLayout->addWidget(myServerConnectionWidget);
            myTopLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));

            {
                buttonOpenFile = new QPushButton(this);
                buttonOpenFile->setCheckable(true);
            }

            myTopLayout->addWidget(buttonOpenFile);
        }
        myMainLayout->addLayout(myTopLayout);

        //options widget
        QHBoxLayout *myOptionsLayout = new QHBoxLayout();
        myOptionsLayout->setContentsMargins(0, 0, 0, 0);
        {
            //message pattern
            {
                labelLoggerPattern = new QLabel(this);
                labelLoggerPattern->setText(tr("Pattern:"));

                lineEditLoggerPattern = new QLineEdit(this);
                lineEditLoggerPattern->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            }

            myOptionsLayout->addWidget(labelLoggerPattern);
            myOptionsLayout->addWidget(lineEditLoggerPattern);
            myOptionsLayout->addSpacing(20);

            //other options
            {
                //TODO put these buttons on different places
                //TODO place filter in search box so it can be edited
                pushButtonClearFilter = new QPushButton(this);
                pushButtonClearFilter->setText(tr("Clear filter"));
                pushButtonClearFilter->setEnabled(false);

                pushButtonClearTable = new QPushButton(this);
                pushButtonClearTable->setText(tr("Clear table"));

                pushButtonSaveToFile = new PushButtonWithMenu(this);
                {
                    QAction *actionSave = new QAction(tr("Save"));
                    actionSave->setData(SAVE);

                    QAction *actionSaveAs = new QAction(tr("Save as"));
                    actionSaveAs->setData(SAVE_AS);

                    QAction *actionSaveAndOpen = new QAction(tr("Save and open"));
                    actionSaveAndOpen->setData(SAVE_AND_OPEN);

                    myActionsSaveToFile.reserve(COUNT_SAVE_TO_FILE);
                    myActionsSaveToFile.insert(SAVE,             actionSave);
                    myActionsSaveToFile.insert(SAVE_AS,          actionSaveAs);
                    myActionsSaveToFile.insert(SAVE_AND_OPEN,    actionSaveAndOpen);

                    pushButtonSaveToFile->addAction(actionSaveAs);
                    pushButtonSaveToFile->addAction(actionSave);
                    pushButtonSaveToFile->addAction(actionSaveAndOpen);
                }

                pushButtonOptions = new QPushButton(this);
                pushButtonOptions->setCheckable(true);
                pushButtonOptions->setText(QStringLiteral("Options"));
            }

            myOptionsLayout->addWidget(pushButtonClearFilter);
            myOptionsLayout->addSpacing(10);
            myOptionsLayout->addWidget(pushButtonClearTable);
            myOptionsLayout->addWidget(pushButtonSaveToFile);
            myOptionsLayout->addSpacing(10);
            myOptionsLayout->addWidget(pushButtonOptions);
        }
        myMainLayout->addLayout(myOptionsLayout);

        //table view, the main view of the widget
        {
            //model view
            myProxyModel = new LoggerTableProxyModel();
            myProxyModel->moveToThread(&myWorkerThread);
            QObject::connect(&myWorkerThread, SIGNAL(finished()), this, SLOT(deleteLater()));
            myWorkerThread.start();

            myTableView = new LoggerTreeView(this);
            myTableView->setAlternatingRowColors(false);
            myTableView->setModel(myProxyModel);
            myTableView->setRootIsDecorated(false);
            myTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            myTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            myTableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            myTableView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            myTableView->setSortingEnabled(false);
            myTableView->header()->setSectionResizeMode(QHeaderView::Interactive);
            myTableView->setContextMenuPolicy(Qt::CustomContextMenu);
            myTableView->setUniformRowHeights(true); //much performance
            myTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        }
        myMainLayout->addWidget(myTableView, INT_MAX);

        mySearchWidget = new SearchWidget(this);
        myMainLayout->addWidget(mySearchWidget);
    }

//    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), myMainLayout->rowCount(), 0);

    this->setLayout(myMainLayout);

    this->setAcceptDrops(true);

    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));
}

void LoggerClientWidget::setupSignalsAndSlots()
{
    //CONNECTION SOCKET
    connect(myChannelSocketClient,      SIGNAL(connectionError(int, QString)),
            this,                       SLOT(connectionError(int, QString)));

    connect(myChannelSocketClient,      SIGNAL(connectionSuccess(QString)),
            this,                       SLOT(connectionSuccess(QString)));

    connect(myChannelSocketClient,      SIGNAL(newMessageReceived(QString)),
            this,                       SLOT(newMessageReceived(QString)));

    connect(myChannelSocketClient,      SIGNAL(newMessageReceived(QString)),
            myProxyModel,               SLOT(newMessageReceived(QString)));

    connect(myChannelSocketClient,      SIGNAL(connectionInProgress()),
            this,                       SLOT(connectionInProgress()));

    //CONNECTION DETAILS
    connect(myServerConnectionWidget,   SIGNAL(buttonConnectToServerToggled(bool)),
            this,                       SLOT(buttonConnectToServerToggled(bool)));

    //FILE
    connect(buttonOpenFile,             SIGNAL(clicked(bool)),
            this,                       SLOT(buttonOpenFileClicked(bool)));

    connect(pushButtonSaveToFile,       SIGNAL(triggered(QAction *)),
            this,                       SLOT(buttonClickedSaveToFile(QAction *)));

    //TABLE EVENTS
    connect(this,                       &LoggerClientWidget::clearModel,
            myProxyModel,               &LoggerTableProxyModel::clear);

    connect(myProxyModel,               &LoggerTableProxyModel::rowsInserted,
            this,                       &LoggerClientWidget::rowsInsertedInModel);

    connect(myProxyModel,               &LoggerTableProxyModel::rowsAboutToBeInserted,
            this,                       &LoggerClientWidget::rowsAboutToBeInsertedInModel);

    connect(myProxyModel,               &LoggerTableProxyModel::rowsRemoved,
            this,                       &LoggerClientWidget::rowsRemovedFromModel);

    connect(this,                       &LoggerClientWidget::parseFile,
            myProxyModel,               &LoggerTableProxyModel::parseFile);

    connect(myProxyModel,               &LoggerTableProxyModel::fileParsingResult,
            this,                       &LoggerClientWidget::fileParsingResult);

    connect(myProxyModel,               &LoggerTableProxyModel::filterStateChanged,
            this,                       &LoggerClientWidget::filterStateChanged);

    connect(this,                       &LoggerClientWidget::parseClipboard,
            myProxyModel,               &LoggerTableProxyModel::parseClipboard);

    connect(myProxyModel,               &LoggerTableProxyModel::clipboardParsingResult,
            this,                       &LoggerClientWidget::clipboardParsingResult);

    connect(myProxyModel,               &LoggerTableProxyModel::showNotification,
    this,                       [ this ](const QString & szMessage, const ToastNotificationWidget::NotificationType eNotifType, const int nTimeoutMs) {
        ToastNotificationWidget::showMessage(this, szMessage, eNotifType, nTimeoutMs);
    });

    //PATTERN
    connect(lineEditLoggerPattern,      SIGNAL(editingFinished()),
            this,                       SLOT(loggerPatternEditingFinished()));

    connect(this,                       SIGNAL(loggerPatternChanged(QString)),
            myProxyModel,               SLOT(setLoggerPattern(QString)));

    connect(myTableView->header(),      SIGNAL(sectionResized(int, int, int)),
            this,                       SLOT(tableViewHeaderResized(int, int, int)));

    connect(myTableView,                SIGNAL(customContextMenuRequested(QPoint)),
            this,                       SLOT(customContextMenuRequestedOnTableView(QPoint)));

    connect(pushButtonClearTable,       SIGNAL(clicked(bool)),
            this,                       SLOT(buttonClickedClearTable(bool)));

    connect(pushButtonClearFilter,      SIGNAL(clicked(bool)),
            this,                       SLOT(buttonClickedClearFilter(bool)));

    //OPTIONS
    connect(pushButtonOptions,          &QPushButton::toggled,
            myOptionsWidget,            &OptionsWidget::setVisible);

    connect(myOptionsWidget,            &OptionsWidget::aboutToHide,
            pushButtonOptions,          [ = ] { pushButtonOptions->setChecked(false); });

    //SEARCH
    connect(mySearchWidget,             SIGNAL(searchTextChanged(QString)),
            this,                       SLOT(searchTextChanged(QString)));

}

void LoggerClientWidget::setupShortcuts()
{
    QShortcut *shortcutPaste            = new QShortcut(QKeySequence(QStringLiteral("Ctrl+V")), this);
    connect(shortcutPaste,              SIGNAL(activated()),
            this,                       SLOT(pasteText()));

    QShortcut *shortcutSearchText       = new QShortcut(QKeySequence(QStringLiteral("Ctrl+F")), this);
    connect(shortcutSearchText,         SIGNAL(activated()),
            mySearchWidget,             SLOT(setFocus()));

    QShortcut *shortcutSearchNext       = new QShortcut(QKeySequence(QStringLiteral("F3")), this);
    connect(shortcutSearchNext,         &QShortcut::activated,
            mySearchWidget,             &SearchWidget::nextResult);

    QShortcut *shortcutSearchPrevious   = new QShortcut(QKeySequence(QStringLiteral("Shift+F3")), this);
    connect(shortcutSearchPrevious,     &QShortcut::activated,
            mySearchWidget,             &SearchWidget::previousResult);
}

void LoggerClientWidget::loadSettings()
{
    QString szDimensions = AppSettings::getValue(AppSettings::KEY_WINDOW_POS_MAIN, "0").toString();
    QVector<QStringRef> szraDimensions = szDimensions.splitRef(GlobalConstants::SETTINGS_STRING_SEPARATOR);

    if (szraDimensions.size() == 4) {
        QPoint myPos(szraDimensions.at(0).toInt(), szraDimensions.at(1).toInt());
        QSize mySize(szraDimensions.at(2).toInt(), szraDimensions.at(3).toInt());

        this->move(myPos);
        this->resize(mySize);

    } else {
        this->resize(600, 800); //default
    }

    QString szServerAddressIpV4 = AppSettings::getValue(AppSettings::KEY_SERVER_IPv4).toString();
    myServerConnectionWidget->setIp(szServerAddressIpV4);

    QString szServerPort = AppSettings::getValue(AppSettings::KEY_SERVER_PORT).toString();
    myServerConnectionWidget->setPort(szServerPort);

    QString szLoggerPattern = AppSettings::getValue(AppSettings::KEY_LOGGER_PATTERN, QStringLiteral("%d %t [%c{1}] %p - %m")).toString();
    lineEditLoggerPattern->setText(szLoggerPattern);
    emit loggerPatternChanged(szLoggerPattern);

}

void LoggerClientWidget::setLogWidgetMode(const LogMode eMode, const QString &szText)
{
    if (eCurrentMode == eMode) {
        return;
    }

    updateButtonsRowCountDependent(eMode);

    switch (eMode) {
        case LoggerClientWidget::EMPTY:
        case LoggerClientWidget::COUNT_LOG_MODE:
            this->setWindowTitle(szWindowTitle);

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::IDLE);

            lineEditLoggerPattern->setEnabled(true);

            buttonOpenFile->setEnabled(true);
            buttonOpenFile->setChecked(false);
            buttonOpenFile->setText(tr("Open log file"));
            break;

        case LoggerClientWidget::CLIPBOARD:
            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + tr("Clipboard log"));

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::IDLE);

            lineEditLoggerPattern->setEnabled(true);

            buttonOpenFile->setChecked(false);
            buttonOpenFile->setText(tr("Open log file"));
            break;

        case LoggerClientWidget::FILE:
            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + tr("File: ") + szText);

            myServerConnectionWidget->setEnabled(false);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::IDLE);

            lineEditLoggerPattern->setEnabled(false);

            buttonOpenFile->setChecked(true);
            buttonOpenFile->setText("Close " + szText);
            break;

        case LoggerClientWidget::SERVER_CONNECTING: {
            QString szInfoMessage = tr("Connecting to: ") + myServerConnectionWidget->getIp() + ':' + myServerConnectionWidget->getPort();
            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + szInfoMessage);

            ToastNotificationWidget::showMessage(this, szInfoMessage, ToastNotificationWidget::INFO, 2000);

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::CONNECTING);

            buttonOpenFile->setEnabled(false);
            break;
        }

        case LoggerClientWidget::SERVER_CONNECTED: {
            QString szInfoMessage = tr("Connected to: ") + myServerConnectionWidget->getIp() + ':' + myServerConnectionWidget->getPort();

            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + szInfoMessage);

            ToastNotificationWidget::showMessage(this, szInfoMessage, ToastNotificationWidget::INFO, 1000);

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::CONNECTED);

            buttonOpenFile->setEnabled(false);
            break;
        }

        case LoggerClientWidget::SERVER_RETRYING: {
            QString szInfoMessage = tr("Disconnected. Retrying: ") + myServerConnectionWidget->getIp() + ':' + myServerConnectionWidget->getPort();

            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + szInfoMessage);

            ToastNotificationWidget::showMessage(this, szInfoMessage, ToastNotificationWidget::INFO, 1000);

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::RETRYING);

            buttonOpenFile->setEnabled(false);
            break;
        }
    }

    eCurrentMode = eMode;

    selectFocus();
}

void LoggerClientWidget::selectFocus()
{
    switch (eCurrentMode) {
        case LoggerClientWidget::EMPTY:
        case LoggerClientWidget::COUNT_LOG_MODE:
            myServerConnectionWidget->setFocus();
            break;

        case LoggerClientWidget::SERVER_CONNECTING:
        case LoggerClientWidget::SERVER_RETRYING:
            myServerConnectionWidget->getButtonConnectToServer()->setFocus();
            break;

        case LoggerClientWidget::CLIPBOARD:
        case LoggerClientWidget::FILE:
        case LoggerClientWidget::SERVER_CONNECTED:
            myTableView->setFocus();
            break;
    }
}

void LoggerClientWidget::updateButtonsRowCountDependent(LogMode eNewMode)
{
    if (myProxyModel->rowCount() == 0) {
        pushButtonSaveToFile->setEnabled(false);
        pushButtonClearTable->setEnabled(false);

    } else {
        pushButtonSaveToFile->setEnabled(true);

        if (eNewMode == COUNT_LOG_MODE) {
            eNewMode = eCurrentMode;
        }

        switch (eNewMode) {
            case LoggerClientWidget::EMPTY:
            case LoggerClientWidget::COUNT_LOG_MODE:
                break;

            case LoggerClientWidget::CLIPBOARD:
                pushButtonClearTable->setEnabled(true); //on Clipboard, clear table is allowed
                return;

            case LoggerClientWidget::FILE:
                pushButtonClearTable->setEnabled(false); //on File, to clear the table the file should be closed
                return;

            case LoggerClientWidget::SERVER_CONNECTING:
            case LoggerClientWidget::SERVER_CONNECTED:
            case LoggerClientWidget::SERVER_RETRYING:
                if (eCurrentMode == LoggerClientWidget::CLIPBOARD
                    || eCurrentMode == LoggerClientWidget::FILE) {

                    emit clearModel();
                }

                pushButtonClearTable->setEnabled(true);

                break;
        }
    }

}

void LoggerClientWidget::checkResizeColumns(bool bIgnoreRowCount)
{
    if (bUsingCustomColumnWidth == true) {
        return;

    } else if (bIgnoreRowCount == true
               || myTableView->model()->rowCount() == 1) {

        myTableView->header()->resizeSections(QHeaderView::ResizeToContents); //don't do this very often, for performance reasons
    }
}

void LoggerClientWidget::saveTableToFile(const QString &szFilename)
{
    //new file to use when saving, update buttons tooltip
    if (szFilename.isEmpty() == false) {
        szSavedLogFile = szFilename;

        QString szFileOnlyName = QFileInfo(szSavedLogFile).fileName();

        myActionsSaveToFile.at(SAVE)->setToolTip(tr("Save") + ' ' + szFileOnlyName);
        myActionsSaveToFile.at(SAVE_AND_OPEN)->setToolTip(tr("Save and open") + ' ' + szFileOnlyName);

        if (bOpenFileAfterSavingPending == true) {
            pushButtonSaveToFile->setDefaultAction(myActionsSaveToFile.at(SAVE_AND_OPEN));

        } else {
            pushButtonSaveToFile->setDefaultAction(myActionsSaveToFile.at(SAVE));
        }
    }

    QFile myFile(szSavedLogFile);

    if (myFile.open(QFile::WriteOnly | QFile::Truncate) == true) {

        QTextStream myTextStream(&myFile);
        myTextStream << myTableView->toString();

        myFile.close();

        ToastNotificationWidget::showMessage(this, tr("File saved successfully"), ToastNotificationWidget::SUCCESS, 2000);

        if (bOpenFileAfterSavingPending == true) {
            bOpenFileAfterSavingPending = false;

            QDesktopServices::openUrl(QUrl(szSavedLogFile));
        }

    } else {
        ToastNotificationWidget::showMessage(this, tr("Saving file failed"), ToastNotificationWidget::ERROR, 2000);
    }
}

void LoggerClientWidget::buttonConnectToServerToggled(bool bButtonState)
{
    if (bButtonState == true) {
        myChannelSocketClient->setNeverDies(true); //TODO get this option from an UI checkbox
        bool bConnectionResult = myChannelSocketClient->connect(myServerConnectionWidget->getIp(), myServerConnectionWidget->getPort());

        if (bConnectionResult == true) {
            setLogWidgetMode(SERVER_CONNECTING);

            AppSettings::setValue(AppSettings::KEY_SERVER_IPv4, myServerConnectionWidget->getIp());
            AppSettings::setValue(AppSettings::KEY_SERVER_PORT, myServerConnectionWidget->getPort());

        } else {
            setLogWidgetMode(EMPTY);

            ToastNotificationWidget::showMessage(this, QStringLiteral("Wrong IP or Port"), ToastNotificationWidget::ERROR, 2000);
        }

    } else {
        setLogWidgetMode(EMPTY);

        myChannelSocketClient->disconnectAndStopRetries();
    }
}

void LoggerClientWidget::buttonOpenFileClicked(bool bButtonState)
{
    if (bButtonState == true) {
        buttonOpenFile->setChecked(false); //keep unchecked

        QFileDialog *myFileDialog = new QFileDialog(this);
        myFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        myFileDialog->setFileMode(QFileDialog::ExistingFile);

        myFileDialog->open(this, SLOT(buttonOpenFileResult(QString)));

    } else {
        setLogWidgetMode(EMPTY);

        emit clearModel();
    }

    selectFocus();
}

void LoggerClientWidget::buttonOpenFileResult(const QString &szFilename)
{
    emit parseFile(szFilename);
}

void LoggerClientWidget::fileParsingResult(bool bParsingResult, const QString &szFilename)
{
    if (bParsingResult == true) {
        setLogWidgetMode(FILE, szFilename);
        checkResizeColumns(true);

    } else {
        setLogWidgetMode(EMPTY);

        ToastNotificationWidget::showMessage(this, tr("Error parsing file"), ToastNotificationWidget::ERROR, 2000);
    }
}

void LoggerClientWidget::pasteText()
{
    emit parseClipboard();
}

void LoggerClientWidget::clipboardParsingResult(bool bParsingResult)
{
    if (bParsingResult == true) {
        setLogWidgetMode(CLIPBOARD);
        checkResizeColumns(true);
        updateButtonsRowCountDependent();

    } else {
        setLogWidgetMode(EMPTY);

        ToastNotificationWidget::showMessage(this, tr("Error parsing clipboard"), ToastNotificationWidget::ERROR, 2000);
    }
}

void LoggerClientWidget::rowsAboutToBeInsertedInModel(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

    if (myTableView->verticalScrollBar()->value() == myTableView->verticalScrollBar()->maximum()
        || myTableView->verticalScrollBar()->isVisible() == false) {

        bIsAtBottom = true;

    } else {
        bIsAtBottom = false;
    }
}

void LoggerClientWidget::rowsInsertedInModel(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(last);

//    qDebug() << "rowsInsertedInModel" << first << last;

    //if at bottom, stay at bottom, otherwise don't move
    if (bIsAtBottom == true) {
        myTableView->scrollToBottom();
    }

    if (first % 100 == 0) {
        updateButtonsRowCountDependent();
        checkResizeColumns();
    }
}

void LoggerClientWidget::rowsRemovedFromModel(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

//    qDebug() << "rowsRemovedFromModel" << first << last;

    if (myTableView->model()->rowCount() == 0) {
        updateButtonsRowCountDependent();
    }
}

void LoggerClientWidget::buttonClickedClearTable(bool bState)
{
    Q_UNUSED(bState)

    emit clearModel();

    updateButtonsRowCountDependent();
    selectFocus();
}

void LoggerClientWidget::buttonClickedClearFilter(bool bState)
{
    Q_UNUSED(bState)

    myProxyModel->setFilterRegExp(QRegExp());

    updateButtonsRowCountDependent();
    selectFocus();
}

void LoggerClientWidget::buttonClickedSaveToFile(QAction *myAction)
{
    if (myAction == nullptr) {
        return;
    }

    bool bChooseFile = false;
    bool bOpenAfterSave = false;

    switch (static_cast<SaveToFileAction>(myAction->data().toInt())) {
        case LoggerClientWidget::SAVE:
        case LoggerClientWidget::COUNT_SAVE_TO_FILE:
            bChooseFile = false;
            bOpenAfterSave = false;
            break;

        case LoggerClientWidget::SAVE_AS:
            bChooseFile = true;
            bOpenAfterSave = false;
            break;

        case LoggerClientWidget::SAVE_AND_OPEN:
            bChooseFile = false;
            bOpenAfterSave = true;
            break;
    }

    if (bOpenAfterSave == true) {
        this->bOpenFileAfterSavingPending = true;
    }

    if (bChooseFile == true
        || this->szSavedLogFile.isEmpty()) {

        QFileDialog *myFileDialog = new QFileDialog(this);
        myFileDialog->setWindowTitle(myAction->text());
        myFileDialog->setAcceptMode(QFileDialog::AcceptSave);
        myFileDialog->setFileMode(QFileDialog::AnyFile);

        myFileDialog->setNameFilters(FileUtils::getFileTypeFilterLog());

        myFileDialog->open(this, SLOT(buttonSaveToFileResult(QString)));

    } else {
        saveTableToFile();
    }

    selectFocus();
}

void LoggerClientWidget::buttonSaveToFileResult(const QString &szFilename)
{
    if (FileUtils::isFileTypeLog(szFilename) == true) {
        saveTableToFile(szFilename);

    } else {
        ToastNotificationWidget::showMessage(this,
                                             FileUtils::getErrorOnlyLogAllowed(),
                                             ToastNotificationWidget::ERROR,
                                             5000);
    }

}

void LoggerClientWidget::customContextMenuRequestedOnTableView(QPoint pos)
{
    QModelIndex myModelIndex = myTableView->indexAt(pos);

    QList<QAction *> *myActionList = myProxyModel->generateActionsForIndex(myModelIndex, this);

    if (myActionList == nullptr) {
        return;
    }

    QMenu *myMenu = new QMenu(this);

    if (myActionList->size() == 0) {
        QAction *myAction = new QAction(tr("No action available"));
        myAction->setEnabled(false);

        myMenu->addAction(myAction);

    } else {
        myMenu->addActions(*myActionList);
    }

    myMenu->popup(myTableView->viewport()->mapToGlobal(pos));
}

void LoggerClientWidget::filterStateChanged(bool bState)
{
    if (bState == true) {
        pushButtonClearFilter->setEnabled(true);

    } else {
        pushButtonClearFilter->setEnabled(false);
    }
}

//TODO filter not applying to new messages
void LoggerClientWidget::searchTextChanged(const QString &szText)
{
//    qDebug() << "searchTextChanged:" << szText;

    myProxyModel->setFilterRegExp(QRegExp(szText, Qt::CaseInsensitive, QRegExp::FixedString));
    myProxyModel->setFilterKeyColumn(-1);

//    QModelIndexList myMatches = myProxyModel->match(myProxyModel->index(0, 0), Qt::UserRole, szText, -1, Qt::MatchContains | Qt::MatchFixedString | Qt::MatchWrap);

//    QStringList matchRows;

//    foreach (const QModelIndex &myIndex, myMatches ) {
////        QTableWidgetItem *item = table->item(myIndex.row(), myIndex.column());
//        matchRows << QString::number(myIndex.row());
//    }

//    qDebug() << "MATCHES:" << matchRows.join(',');
}

void LoggerClientWidget::connectionSuccess(const QString &szError)
{
    Q_UNUSED(szError)

    setLogWidgetMode(SERVER_CONNECTED);
}

void LoggerClientWidget::connectionError(int nSocketError, const QString &szError)
{
    qDebug() << __FUNCTION__ << nSocketError << szError;

    setLogWidgetMode(SERVER_RETRYING);
}

void LoggerClientWidget::connectionInProgress()
{
    setLogWidgetMode(SERVER_RETRYING);
}

void LoggerClientWidget::newMessageReceived(const QString &szMessage)
{
    Q_UNUSED(szMessage)

//    myTableView->viewport()->update();
}

void LoggerClientWidget::loggerPatternEditingFinished()
{
    QString szLoggerPattern = lineEditLoggerPattern->text();

    emit loggerPatternChanged(szLoggerPattern);

    AppSettings::setValue(AppSettings::KEY_LOGGER_PATTERN, szLoggerPattern);
}

void LoggerClientWidget::tableViewHeaderResized(int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(logicalIndex)
    Q_UNUSED(oldSize)
    Q_UNUSED(newSize)

    if (myProxyModel->rowCount() != 0) {
        bUsingCustomColumnWidth = true;
    }
}

void LoggerClientWidget::dragEnterEvent(QDragEnterEvent *myDragEvent)
{
    if (myDragEvent->mimeData()->hasText()) {
        myDragEvent->acceptProposedAction();

    } else {
        myDragEvent->ignore();
    }
}

void LoggerClientWidget::dropEvent(QDropEvent *myDropEvent)
{
    if (myDropEvent->mimeData()->hasText() == true) {
        const QMimeData *mime = myDropEvent->mimeData();
        QString szDroppedFile = mime->text();

        if (szDroppedFile.startsWith(QLatin1String("file:///")) == true) {
            szDroppedFile.remove(0, QLatin1String("file:///").size());
        }

        emit parseFile(szDroppedFile);

        myDropEvent->accept();
        return;

    } else {
        myDropEvent->ignore();
    }

    QWidget::dropEvent(myDropEvent);
}

void LoggerClientWidget::resizeEvent(QResizeEvent *event)
{
    saveWindowPosition();
    QWidget::resizeEvent(event);
}

void LoggerClientWidget::moveEvent(QMoveEvent *event)
{
    saveWindowPosition();
    QWidget::moveEvent(event);
}

#ifdef DEBUG_STUFF
void LoggerClientWidget::initDebugStuff()
{
    ///DEBUG
    connect(qApp,                       &QApplication::focusChanged,
    this,                       [ ] (QWidget * before, QWidget * after) {
        qDebug() << "Focus changed"
                 << "from"
                 << (before == nullptr ? "nullptr" : before->metaObject()->className())
                 << "to"
                 << (after == nullptr ? "nullptr" : after->metaObject()->className());
    });
}
#endif


