#include "LoggerClientWidget.h"
#include "application/AppSettings.h"
#include "data/LoggerTableProxyModel.h"
#include "data/LoggerTableItemModel.h"
#include "network/ChannelSocketClient.h"
#include "ui/ToastNotificationWidget.h"
#include "ui/element/PushButtonWithMenu.h"
#include "ui/modal/OptionsWidget.h"
#include "ui/widget/KeywordHighlightWidget.h"
#include "ui/widget/LoggerPatternWidget.h"
#include "ui/widget/NetworkConnectionWidget.h"
#include "ui/widget/SearchWidget.h"
#include "util/FileUtils.h"
#include "util/MemoryUtils.h"
#include "util/TimeUtils.h"
#include "view/LoggerTreeView.h"

//TODO Bug: filter by one column. Then filter by other column, Clear Table button becomes disabled
//TODO add option to color by thread
//TODO class filter and text search are mutually exclusive. Both act on setFilterRegExp
//TODO open file and set breakpoint
//TODO open multiple editor context menu
//TODO right click on selected cells, copy only that column contents. Good to export XML from multiple lines
//TODO right click with multiple lines selected: generate "filter by" action for all classes
//TODO add default name for log file saving as text
//TODO add TipWidget (NAM: you can copy and paste from the table. Paste in the format...). Auto hides itself by a checkbox in the OptionsWidget
//TODO add button to reset patterns to default values

const int LoggerClientWidget::CLEAR_UNDO_TIMEOUT_MS;

LoggerClientWidget::LoggerClientWidget(QWidget *parent)
    : QWidget(parent)
    , IntMutexable()
    , bUsingCustomColumnWidth(false)
    , eCurrentMode(COUNT_LOG_MODE)
    , bOpenFileAfterSavingPending(false)
    , bIsAtBottom(true)
    , mySavedModelIndex()
    , myTimerUndoClear(new QTimer(this))
    , bIsClearPending(false)
    , myOptionsWidget(new OptionsWidget(this)) //loads current settings on construction. Should load when calling method, but it's good enough for now
{
#ifdef DEBUG_STUFF
//    initDebugFocusChanged();
#endif

    this->hide();

    myChannelSocketClient = new ChannelSocketClient();

    myTimerUndoClear->setSingleShot(true);

    setupUI();
    setStyleSheet(QString());
    setupSignalsAndSlots();
    setupShortcuts();
    loadSettings();

    setLogWidgetMode(EMPTY);

    this->show();
}

LoggerClientWidget::~LoggerClientWidget()
{
    if (myTimerUndoClear->isActive()) {
        myTimerUndoClear->stop();
        finalizeClearTable();
    }

    MemoryUtils::deletePointer(myProxyModel);

    myWorkerThread.quit();
    myWorkerThread.wait(200);
}

void LoggerClientWidget::setStyleSheet(const QString &szStyleSheet)
{
    QWidget::setStyleSheet(szStyleSheet);
}

void LoggerClientWidget::saveWindowPosition()
{
    const QPoint myPos = this->pos();
    const QSize mySize = this->size();

    AppSettings::setValue(AppSettings::KEY_WINDOW_POS_MAIN,
                          QString::number(myPos.x())
                          + GlobalConstants::SEPARATOR_SETTINGS_LIST + QString::number(myPos.y())
                          + GlobalConstants::SEPARATOR_SETTINGS_LIST + QString::number(mySize.width())
                          + GlobalConstants::SEPARATOR_SETTINGS_LIST + QString::number(mySize.height())
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
            myLoggerPatternWidget = new LoggerPatternWidget(this);

            myOptionsLayout->addWidget(myLoggerPatternWidget);
            myOptionsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

            //other options
            {
                //TODO put these buttons on different places
                //TODO place filter in search box so it can be edited
                pushButtonResizeColumns = new QPushButton(this);
                pushButtonResizeColumns->setText(tr("&Resize"));
                pushButtonResizeColumns->setEnabled(false);

                pushButtonClearTable = new QPushButton(this);
                pushButtonClearTable->setText(tr("Cl&ear table"));

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
                pushButtonOptions->setText(QStringLiteral("&Options"));
            }

            myOptionsLayout->addWidget(pushButtonResizeColumns);
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
//            myTableView->setRootIsDecorated(false);
            myTableView->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            myTableView->setSortingEnabled(false);
            myTableView->setShowGrid(false);
            myTableView->setContextMenuPolicy(Qt::CustomContextMenu);
//            myTableView->setUniformRowHeights(true); //much performance for QTreeView
            myTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
            myTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
//            myTableView->setAutoScroll(false);
            myTableView->setWordWrap(false);

            //Horizontal
            myTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            myTableView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            myTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
            myTableView->horizontalHeader()->setHighlightSections(false);
            myTableView->horizontalHeader()->setStretchLastSection(true);
            //Vertical
            myTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            myTableView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            myTableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
            myTableView->verticalHeader()->setVisible(false);
        }
        myMainLayout->addWidget(myTableView, INT_MAX);

        //filters
        QHBoxLayout *myFiltersLayout = new QHBoxLayout();
        myFiltersLayout->setContentsMargins(0, 0, 0, 0);
        {
            mySearchWidget = new SearchWidget(this);

            pushButtonClearFilter = new QPushButton(this);
            pushButtonClearFilter->setText(tr("Clear &filter"));
            pushButtonClearFilter->setEnabled(false);

            myKeywordHighlightWidget = new KeywordHighlightWidget(this);

            myFiltersLayout->addWidget(mySearchWidget);
            myFiltersLayout->addWidget(pushButtonClearFilter);
            myFiltersLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
            myFiltersLayout->addWidget(myKeywordHighlightWidget);
        }
        myMainLayout->addLayout(myFiltersLayout);
    }

//    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), myMainLayout->rowCount(), 0);

    this->setLayout(myMainLayout);

    this->setAcceptDrops(true);

    this->setWindowIcon(QIcon(QStringLiteral(":/icons/themes/icons/appIcon.svg")));
}

void LoggerClientWidget::setupSignalsAndSlots()
{
    //CONNECTION SOCKET
    connect(myChannelSocketClient,      SIGNAL(connectionError(int, QString)),
            this,                       SLOT(connectionError(int, QString)));

    connect(myChannelSocketClient,      SIGNAL(connectionSuccess(QString)),
            this,                       SLOT(connectionSuccess(QString)));

//    connect(myChannelSocketClient,      SIGNAL(newMessageReceived(QString)),
//            this,                       SLOT(newMessageReceived(QString)));

    connect(myChannelSocketClient,      SIGNAL(newMessageReceived(QString)),
            myProxyModel,               SLOT(newMessageReceived(QString)));

    connect(myChannelSocketClient,      SIGNAL(connectionInProgress()),
            this,                       SLOT(connectionInProgress()));

    //CONNECTION DETAILS
    connect(myServerConnectionWidget,   SIGNAL(signalButtonConnectToServerToggled(bool)),
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
    connect(myLoggerPatternWidget,      &LoggerPatternWidget::loggerPatternChangedSignal,
            this,                       &LoggerClientWidget::loggerPatternChanged);

    connect(this,                       &LoggerClientWidget::loggerPatternChangedSignal,
            myProxyModel,               &LoggerTableProxyModel::setLoggerPattern);

    connect(myTableView->horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
            this,                       SLOT(tableViewHeaderResized(int, int, int)));

    connect(myTableView,                SIGNAL(customContextMenuRequested(QPoint)),
            this,                       SLOT(customContextMenuRequestedOnTableView(QPoint)));

    connect(pushButtonResizeColumns,    &QPushButton::clicked,
            this,                       &LoggerClientWidget::resizeColumnsLoosely);

    connect(pushButtonClearTable,       SIGNAL(clicked(bool)),
            this,                       SLOT(buttonClickedClearTable(bool)));

    //OPTIONS
    connect(pushButtonOptions,          &QPushButton::toggled,
            myOptionsWidget,            &OptionsWidget::setVisible);

    connect(myOptionsWidget,            &OptionsWidget::aboutToHide,
            pushButtonOptions,          [ this ] { pushButtonOptions->setChecked(false); pushButtonOptions->clearFocus(); });

    connect(myOptionsWidget,            &OptionsWidget::fontSizeChanged,
            this,                       &LoggerClientWidget::fontSizeChanged);

    connect(myOptionsWidget,            &OptionsWidget::rowHeightBiasChanged,
            this,                       &LoggerClientWidget::rowHeightBiasChanged);

    //SEARCH
    connect(mySearchWidget,             &SearchWidget::searchTextChanged,
            this,                       &LoggerClientWidget::searchTextChanged);

    connect(pushButtonClearFilter,      SIGNAL(clicked(bool)),
            this,                       SLOT(buttonClickedClearFilter(bool)));

    connect(myKeywordHighlightWidget,   &KeywordHighlightWidget::keywordsChangedSignal,
            this,                       &LoggerClientWidget::keywordHighlightChanged);

    // UNDO TIMER
    connect(myTimerUndoClear,           &QTimer::timeout,
            this,                       &LoggerClientWidget::finalizeClearTable);
}

void LoggerClientWidget::setupShortcuts()
{
    QShortcut *shortcutPaste            = new QShortcut(QKeySequence(QStringLiteral("Ctrl+V")), this);
    connect(shortcutPaste,              &QShortcut::activated,
            this,                       &LoggerClientWidget::pasteText);


    QShortcut *shortcutFontUp           = new QShortcut(QKeySequence(QStringLiteral("Ctrl++")), this, nullptr, nullptr, Qt::ApplicationShortcut);
    connect(shortcutFontUp,             &QShortcut::activated,
            this,                       [ this ] { qDebug() << "zooming"; myOptionsWidget->fontSizeChange(1);});

    QShortcut *shortcutFontDown         = new QShortcut(QKeySequence(QStringLiteral("Ctrl+-")), this, nullptr, nullptr, Qt::ApplicationShortcut);
    connect(shortcutFontDown,           &QShortcut::activated,
            this,                       [ this ] { qDebug() << "zooming"; myOptionsWidget->fontSizeChange(-1);});

    QShortcut *shortcutFontReset        = new QShortcut(QKeySequence(QStringLiteral("Ctrl+0")), this, nullptr, nullptr, Qt::ApplicationShortcut);
    connect(shortcutFontReset,           &QShortcut::activated,
            this,                       [ this ] { qDebug() << "zooming"; myOptionsWidget->fontSizeChange(0);});

    QShortcut *shortcutRowHeightUp      = new QShortcut(QKeySequence(QStringLiteral("Alt++")), this, nullptr, nullptr, Qt::ApplicationShortcut);
    connect(shortcutRowHeightUp,        &QShortcut::activated,
            this,                       [ this ] { qDebug() << "zooming"; myOptionsWidget->rowHeightBiasChange(1);});

    QShortcut *shortcutRowHeightDown    = new QShortcut(QKeySequence(QStringLiteral("Alt+-")), this, nullptr, nullptr, Qt::ApplicationShortcut);
    connect(shortcutRowHeightDown,      &QShortcut::activated,
            this,                       [ this ] { qDebug() << "zooming"; myOptionsWidget->rowHeightBiasChange(-1);});

    QShortcut *shortcutRowHeightReset   = new QShortcut(QKeySequence(QStringLiteral("Alt+0")), this, nullptr, nullptr, Qt::ApplicationShortcut);
    connect(shortcutRowHeightReset,      &QShortcut::activated,
            this,                       [ this ] { qDebug() << "zooming"; myOptionsWidget->rowHeightBiasChange(0);});
}

void LoggerClientWidget::loadSettings()
{
    const QString szDimensions = AppSettings::getValue(AppSettings::KEY_WINDOW_POS_MAIN, "0").toString();
    const QList<QStringView> szraDimensions = QStringView(szDimensions).split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

    if (szraDimensions.size() == 4) {
        const QPoint myPos(szraDimensions.at(0).toInt(), szraDimensions.at(1).toInt());
        const QSize mySize(szraDimensions.at(2).toInt(), szraDimensions.at(3).toInt());

        this->move(myPos);
        this->resize(mySize);

    } else {
        this->resize(600, 800); //default
    }

    const QString szServerName = AppSettings::getValue(AppSettings::KEY_SERVER_NAME).toString();
    myServerConnectionWidget->setName(szServerName);

    const QString szServerIpV4 = AppSettings::getValue(AppSettings::KEY_SERVER_IPv4).toString();
    myServerConnectionWidget->setIp(szServerIpV4);

    const QString szServerPort = AppSettings::getValue(AppSettings::KEY_SERVER_PORT).toString();
    myServerConnectionWidget->setPort(szServerPort);

    loggerPatternChanged(myLoggerPatternWidget->getPattern());

    const int nRowBias = AppSettings::getValue(AppSettings::KEY_ROW_HEIGHT_BIAS, 0).toInt();
    rowHeightBiasChanged(nRowBias);

    keywordHighlightChanged(myKeywordHighlightWidget->getKeywords());
}

void LoggerClientWidget::setLogWidgetMode(const LogMode eMode, const QString &szText, bool bForce)
{
    // If a clear is pending when the mode changes, finalize it immediately
    if (bIsClearPending) {
        finalizeClearTable();
    }

    if (bForce == false
        && eCurrentMode == eMode) {
        return;
    }

    updateButtonsRowCountDependent(eMode);

    switch (eMode) {
        case LoggerClientWidget::EMPTY:
        case LoggerClientWidget::COUNT_LOG_MODE:
            this->setWindowTitle(szWindowTitle);

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::IDLE);

            myLoggerPatternWidget->setEnabled(true);

            buttonOpenFile->setEnabled(true);
            buttonOpenFile->setChecked(false);
            buttonOpenFile->setText(tr("Open &log file"));
            break;

        case LoggerClientWidget::CLIPBOARD:
            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + tr("Clipboard log"));

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::IDLE);

            myLoggerPatternWidget->setEnabled(true);

            buttonOpenFile->setChecked(false);
            buttonOpenFile->setText(tr("Open &log file"));
            break;

        case LoggerClientWidget::FILE:
            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + tr("File: ") + szText);

            myServerConnectionWidget->setEnabled(false);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::IDLE);

            myLoggerPatternWidget->setEnabled(true);

            buttonOpenFile->setChecked(true);
            buttonOpenFile->setText("C&lose " + szText);
            break;

        case LoggerClientWidget::SERVER_CONNECTING: {
            const QString szInfoMessage = tr("Connecting to: ") + getClientInfoMessage();

            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + szInfoMessage);

            ToastNotificationWidget::showMessage(this, szInfoMessage, ToastNotificationWidget::INFO, 2000);

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::CONNECTING);

            buttonOpenFile->setEnabled(false);
            break;
        }

        case LoggerClientWidget::SERVER_CONNECTED: {
            const QString szInfoMessage = tr("Connected to: ") + getClientInfoMessage();

            this->setWindowTitle(szWindowTitle + QStringLiteral(" - ") + szInfoMessage);

            ToastNotificationWidget::showMessage(this, szInfoMessage, ToastNotificationWidget::INFO, 1000);

            myServerConnectionWidget->setEnabled(true);
            myServerConnectionWidget->setMode(NetworkConnectionWidget::CONNECTED);

            buttonOpenFile->setEnabled(false);

            if (mySearchWidget->isEmpty() == false) {
                mySearchWidget->triggerSearchTextChanged(); //reapply filter

            } else {
                myProxyModel->reApplyFilter(); //will reapply column filter if it exists
            }

            break;
        }

        case LoggerClientWidget::SERVER_RETRYING: {
            const QString szInfoMessage = tr("Disconnected. Retrying: ") + getClientInfoMessage();

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
    updateClearButtonText();

    // Handle pending clear state first
    if (bIsClearPending) {
        pushButtonResizeColumns->setEnabled(false); // Can't resize while pending
        pushButtonSaveToFile->setEnabled(false);    // Can't save while pending
        pushButtonClearTable->setEnabled(true);     // Keep Undo enabled
        return; // Don't process further if pending
    }

    if (myProxyModel->rowCount() == 0) {
        pushButtonResizeColumns->setEnabled(false);
        pushButtonSaveToFile->setEnabled(false);
        pushButtonClearTable->setEnabled(false);

    } else {
        pushButtonResizeColumns->setEnabled(true);
        pushButtonSaveToFile->setEnabled(true);

        if (eNewMode == COUNT_LOG_MODE) {
            eNewMode = eCurrentMode;
        }

        switch (eNewMode) {
            case LoggerClientWidget::EMPTY:
            case LoggerClientWidget::COUNT_LOG_MODE:
                // Should not happen if rowCount > 0, but disable clear just in case
                pushButtonClearTable->setEnabled(false);
                break;

            case LoggerClientWidget::CLIPBOARD:
                pushButtonClearTable->setEnabled(true); //on Clipboard, clear table is allowed
                break;

            case LoggerClientWidget::FILE:
                pushButtonClearTable->setEnabled(false); //on File, to clear the table the file should be closed
                break;

            case LoggerClientWidget::SERVER_CONNECTING:
            case LoggerClientWidget::SERVER_CONNECTED:
            case LoggerClientWidget::SERVER_RETRYING:
                pushButtonClearTable->setEnabled(true);
                break;
        }
    }
}

void LoggerClientWidget::updateClearButtonText()
{
    if (bIsClearPending) {
        int nSecondsLeft = myTimerUndoClear->remainingTime() / 1000;

        QTimer::singleShot(1000, this, [this] {
            updateClearButtonText();
        });

        if (nSecondsLeft > 0) {
            pushButtonClearTable->setText(tr("&Undo Clear") + " (" + QString::number(nSecondsLeft) + ")");
        }

    } else {
        pushButtonClearTable->setText(tr("Cl&ear table"));
    }
}

void LoggerClientWidget::resizeColumnsIfNeeded(bool bIgnoreRowCount)
{
    if (bUsingCustomColumnWidth == true) {
        return;
    }

    if (bIgnoreRowCount == true
        || myTableView->model()->rowCount() == 1) {

        myTableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents); //don't do this very often, for performance reasons
    }
}

void LoggerClientWidget::saveTableToFile(const QString &szFilename)
{
    //new file to use when saving, update buttons tooltip
    if (szFilename.isEmpty() == false) {
        szSavedLogFile = szFilename;

        const QString szFileOnlyName = QFileInfo(szSavedLogFile).fileName();

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

QString LoggerClientWidget::getClientInfoMessage()
{
    if (myServerConnectionWidget->getName().isEmpty() == true) {
        return myServerConnectionWidget->getIp() + ':' + myServerConnectionWidget->getPort();

    } else {
        return myServerConnectionWidget->getName() + tr(" on ") + myServerConnectionWidget->getIp() + ':' + myServerConnectionWidget->getPort();
    }
}

void LoggerClientWidget::initiateClearTable()
{
    if (bIsClearPending
        || myProxyModel->rowCount() == 0) {
        return;
    }

    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(myProxyModel->sourceModel());

    if (sourceModel == nullptr) {
        qWarning() << "Could not get QStandardItemModel from proxy model!";
        // emit clearModel(); // Old way - uncomment if fallback needed
        updateButtonsRowCountDependent();
        selectFocus();

    } else {
        myListUndoBuffer.clear();

        // Backup rows by taking them from the model
        myListUndoBuffer.reserve(sourceModel->rowCount());

        // Iterate backwards because takeRow shifts subsequent row indices
        // myListUndoBuffer will store rows in their reverse order
        for (int nIndex = sourceModel->rowCount() - 1; nIndex >= 0; --nIndex) {
            myListUndoBuffer.push_back(sourceModel->takeRow(nIndex));
        }

        bIsClearPending = true;
        updateButtonsRowCountDependent(); // Update button text and state

        ToastNotificationWidget::showMessage(this, tr("Table cleared. Click 'Undo Clear' to restore."), ToastNotificationWidget::INFO, CLEAR_UNDO_TIMEOUT_MS);
        myTimerUndoClear->start(CLEAR_UNDO_TIMEOUT_MS);
    }
}

void LoggerClientWidget::savedSelectedIndex()
{
    QModelIndex currentProxyIndex = myTableView->currentIndex();

    if (currentProxyIndex.isValid()) {
        mySavedModelIndex = myProxyModel->mapToSource(currentProxyIndex);

    } else {
        mySavedModelIndex = QPersistentModelIndex(); // Clear if nothing selected
    }
}

void LoggerClientWidget::buttonConnectToServerToggled(bool bButtonState)
{
    if (bButtonState == true) {
        myChannelSocketClient->setNeverDies(true); //TODO get this option from an UI checkbox
        const bool bConnectionResult = myChannelSocketClient->connect(myServerConnectionWidget->getIp(), myServerConnectionWidget->getPort());

        if (bConnectionResult == true) {
            setLogWidgetMode(SERVER_CONNECTING);

            AppSettings::setValue(AppSettings::KEY_SERVER_NAME, myServerConnectionWidget->getName());
            AppSettings::setValue(AppSettings::KEY_SERVER_IPv4, myServerConnectionWidget->getIp());
            AppSettings::setValue(AppSettings::KEY_SERVER_PORT, myServerConnectionWidget->getPort());

        } else {
            setLogWidgetMode(EMPTY);

            ToastNotificationWidget::showMessage(this, tr("Wrong IP or Port"), ToastNotificationWidget::ERROR, 2000);
        }

    } else {
        setLogWidgetMode(EMPTY);

        myChannelSocketClient->disconnectAndStopRetries();

        myProxyModel->resetIndex();
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

        Q_EMIT clearModel();
    }

    selectFocus();
}

void LoggerClientWidget::buttonOpenFileResult(const QString &szFilename)
{
    Q_EMIT parseFile(szFilename);
}

void LoggerClientWidget::fileParsingResult(const int nResult, const QString &szFilename)
{
    if (nResult == GlobalConstants::SUCCESS) {
        setLogWidgetMode(FILE, szFilename, true);
        resizeColumnsIfNeeded(true);

    } else {
        setLogWidgetMode(EMPTY);

        ToastNotificationWidget::showMessage(this, tr("Error parsing file"), ToastNotificationWidget::ERROR, 2000);
    }
}

void LoggerClientWidget::pasteText()
{
    if (eCurrentMode != EMPTY
        && eCurrentMode != CLIPBOARD) {
        if (eCurrentMode == FILE) {
            ToastNotificationWidget::showMessage(this, tr("Can't paste now") + ". " + tr("Close file first"), ToastNotificationWidget::INFO, 3000);

        } else {
            ToastNotificationWidget::showMessage(this, tr("Can't paste now") + ". " + tr("Disconnect first"), ToastNotificationWidget::INFO, 3000);
        }

    } else {
        Q_EMIT parseClipboard();
    }
}

void LoggerClientWidget::clipboardParsingResult(const GlobalConstants::ErrorCode eParsingResult)
{
    if (eParsingResult == GlobalConstants::SUCCESS) {
        setLogWidgetMode(CLIPBOARD);
        resizeColumnsIfNeeded(true);
        updateButtonsRowCountDependent();

        ToastNotificationWidget::showMessage(this, tr("Clipboard parsed successfully"), ToastNotificationWidget::SUCCESS, 1000);

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

    // Update buttons only if not in pending clear state
    if (bIsClearPending == false) {
        if (first % 100 == 0) {
            updateButtonsRowCountDependent();
            resizeColumnsIfNeeded();

        } else if (myProxyModel->rowCount() > 0) {
            updateButtonsRowCountDependent();
        }
    }
}

void LoggerClientWidget::rowsRemovedFromModel(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);

//    qDebug() << "rowsRemovedFromModel" << first << last;

    // Update buttons only if not in pending clear state
    if (bIsClearPending == false
        && myTableView->model()->rowCount() == 0) {

        updateButtonsRowCountDependent();
    }
}

void LoggerClientWidget::buttonClickedClearTable(bool bState)
{
    Q_UNUSED(bState)

    if (bIsClearPending) {
        undoClearTable();

    } else {
        initiateClearTable();
    }

    selectFocus();
}

void LoggerClientWidget::finalizeClearTable()
{
    if (bIsClearPending == false) {
        return;
    }

    // Permanently delete the buffered items
    // MemoryUtils::clearAndDeleteItems(m_undoBuffer);
    myListUndoBuffer.clear();

    bIsClearPending = false;
    updateButtonsRowCountDependent(); // Reset button text and state
    selectFocus();
}

void LoggerClientWidget::undoClearTable()
{
    if (bIsClearPending == false) {
        return;
    }

    myTimerUndoClear->stop();

    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(myProxyModel->sourceModel());

    if (sourceModel == nullptr) {
        qWarning() << "Could not get QStandardItemModel from proxy model during undo";
        // Clear buffer anyway to prevent memory leaks, but can't restore
        myListUndoBuffer.clear();
        bIsClearPending = false;
        updateButtonsRowCountDependent();
        selectFocus();

    } else {
        for (int nIndex = myListUndoBuffer.size() - 1; nIndex >= 0; --nIndex) {
            sourceModel->appendRow(myListUndoBuffer[nIndex]);
        }

        myListUndoBuffer.clear();

        bIsClearPending = false;
        updateButtonsRowCountDependent(); // Reset button text and state
        selectFocus();

        ToastNotificationWidget::showMessage(this, tr("Clear undone. Table restored."), ToastNotificationWidget::SUCCESS, 2000);
    }
}

void LoggerClientWidget::buttonClickedClearFilter(bool bState)
{
    Q_UNUSED(bState)

    savedSelectedIndex();

    mySearchWidget->clear(); //this will trigger searchTextChanged, which calls setFilterRegExp
    myProxyModel->setFilterRegExp(QRegularExpression(), false); // Explicitly clear any remaining filter (e.g., column filter)

    // updateButtonsRowCountDependent(); // filterStateChanged will be emitted, no need to call this here
    // selectFocus(); // Let filterStateChanged handle focus/scrolling
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

void LoggerClientWidget::customContextMenuRequestedOnTableView(const QPoint &pos)
{
    const QModelIndex myModelIndex = myTableView->indexAt(pos);

    QList<QAction *> *myActionList = myProxyModel->generateActionsForIndex(myModelIndex, this);

    if (myActionList == nullptr) {
        return;
    }

    QMenu *myMenu = new QMenu(this);

    if (myActionList->isEmpty() == true) {
        QAction *myAction = new QAction(tr("No action available"));
        myAction->setEnabled(false);

        myMenu->addAction(myAction);

    } else {
        myMenu->addActions(*myActionList);
    }

    myMenu->popup(myTableView->viewport()->mapToGlobal(pos));
}

void LoggerClientWidget::filterStateChanged(bool bIsFilterActive)
{
    pushButtonClearFilter->setEnabled(bIsFilterActive);

    if (mySavedModelIndex.isValid()) {
        QModelIndex newProxyIndex = myProxyModel->mapFromSource(mySavedModelIndex);

        if (newProxyIndex.isValid()) {
            QApplication::processEvents(QEventLoop::WaitForMoreEvents); // update the tableview before scrolling
            myTableView->setCurrentIndex(newProxyIndex);
            myTableView->scrollTo(newProxyIndex, QAbstractItemView::EnsureVisible);
        }

        // Clear the preserved index after attempting restoration
        mySavedModelIndex = QPersistentModelIndex();
    }
}

void LoggerClientWidget::searchTextChanged(const QString &szSearchText, QRegularExpression::PatternOptions eOptions)
{
    //    qDebug() << "searchTextChanged:" << szSearchText;

    savedSelectedIndex();

    QRegularExpression myRegExp(szSearchText, eOptions);

    if (myRegExp.isValid() == true) {
        // myProxyModel->setFilterKeyColumn(myProxyModel->getVisibleIndexForColumn(LoggerEnum::COLUMN_MESSAGE));
        myProxyModel->setFilterKeyColumn(-1);
        myProxyModel->setFilterRegExp(myRegExp, false); //false = not an actual filter, just text search

    } else {
        // ToastNotificationWidget::showMessage(this, tr("Invalid regex"), ToastNotificationWidget::ERROR, 2000);
        myProxyModel->setFilterRegExp(QRegularExpression(), false); // Clear filter on invalid regex
    }

//    QModelIndexList myMatches = myProxyModel->match(myProxyModel->index(0, 0), Qt::UserRole, szText, -1, Qt::MatchContains | Qt::MatchFixedString | Qt::MatchWrap);

//    QStringList matchRows;

//    for (const QModelIndex &myIndex : myMatches) {
////        QTableWidgetItem *item = table->item(myIndex.row(), myIndex.column());
//        matchRows << QString::number(myIndex.row());
//    }

    //    qDebug() << "MATCHES:" << matchRows.join(',');
}

void LoggerClientWidget::keywordHighlightChanged(const QStringList &szaKeywords)
{
    myProxyModel->updateKeywords(szaKeywords);
    myTableView->repaint();
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

    myProxyModel->resetIndex();
}

void LoggerClientWidget::connectionInProgress()
{
    setLogWidgetMode(SERVER_RETRYING);
}

//void LoggerClientWidget::newMessageReceived(const QString &szMessage)
//{
//    Q_UNUSED(szMessage)

////    myTableView->viewport()->update();
//}

void LoggerClientWidget::loggerPatternChanged(const QString &szLoggerPattern)
{
    Q_EMIT loggerPatternChangedSignal(szLoggerPattern);
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

void LoggerClientWidget::resizeColumnsLoosely()
{
    const double dAverageCharWidth = this->font().pointSize() * 0.70; //this->fontMetrics().averageCharWidth() * 1.1;

    for (int nColumn = 0; nColumn < myTableView->horizontalHeader()->count(); ++nColumn) {
        const int nSize = myTableView->getColumnMaxCharCount(nColumn, 0, -1, true);

        myTableView->horizontalHeader()->resizeSection(nColumn, static_cast<int>(qRound(nSize * dAverageCharWidth) + dAverageCharWidth * 3));
    }
}

void LoggerClientWidget::fontSizeChanged(const int nValue)
{
    Q_UNUSED(nValue)

    rowHeightBiasChanged(); //row height depends on font size
}

void LoggerClientWidget::rowHeightBiasChanged(int nValue)
{
    if (nValue == INT_MAX) {
        nValue = AppSettings::getValue(AppSettings::KEY_ROW_HEIGHT_BIAS, 0).toInt(); //reading current value from verticalHeader doesn't work
    }

    myTableView->verticalHeader()->setMinimumSectionSize(myTableView->fontMetrics().height() + 4 + nValue * 2);
    myTableView->verticalHeader()->setDefaultSectionSize(myTableView->verticalHeader()->minimumSectionSize());
    myTableView->verticalHeader()->setMaximumSectionSize(myTableView->verticalHeader()->minimumSectionSize());
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

        if (szDroppedFile.startsWith(QStringLiteral("file:///")) == true) {
            szDroppedFile.remove(0, QStringLiteral("file:///").size());
        }

        Q_EMIT parseFile(szDroppedFile);

        myDropEvent->accept();

    } else {
        myDropEvent->ignore();
        QWidget::dropEvent(myDropEvent);
    }
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
void LoggerClientWidget::initDebugFocusChanged()
{
    ///DEBUG
    connect(qApp,               &QApplication::focusChanged,
    this,                       [ ] (QWidget * before, QWidget * after) {
        qDebug() << "Focus changed"
                 << "from"
                 << (before == nullptr ? "nullptr" : before->metaObject()->className())
                 << "to"
                 << (after == nullptr ? "nullptr" : after->metaObject()->className());
    });
}
#endif
