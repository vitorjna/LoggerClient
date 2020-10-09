#pragma once

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QStyleFactory>
#include <QThread>
#include <QWidget>

#include "application/GlobalConstants.h"
#include "interface/IntMutexable.h"

class ChannelSocketClient;
class KeywordHighlightWidget;
class LoggerPatternWidget;
class LoggerTableProxyModel;
class LoggerTreeView;
class NetworkConnectionWidget;
class OptionsWidget;
class PushButtonWithMenu;
class SearchWidget;

class LoggerClientWidget : public QWidget, public IntMutexable
{
    Q_OBJECT
    QThread myWorkerThread;

    enum LogMode {
        EMPTY,
        CLIPBOARD,
        FILE,
        SERVER_CONNECTING,
        SERVER_CONNECTED,
        SERVER_RETRYING,
        COUNT_LOG_MODE
    };

    enum SaveToFileAction {
        SAVE,
        SAVE_AS,
        SAVE_AND_OPEN,
        COUNT_SAVE_TO_FILE
    };

public:
    explicit LoggerClientWidget(QWidget *parent = nullptr);
    ~LoggerClientWidget() override;

    void setStyleSheet(const QString &szStyleSheet);

protected:
    void saveWindowPosition();

private:
    void setupUI();
    void setupSignalsAndSlots();
    void setupShortcuts();
    void loadSettings();

    void setLogWidgetMode(const LogMode eMode, const QString &szText = QStringLiteral(""), bool bForce = false);

    void selectFocus();
    void updateButtonsRowCountDependent(LogMode eNewMode = COUNT_LOG_MODE);
    void resizeColumnsIfNeeded(bool bIgnoreRowCount = false);

    void saveTableToFile(const QString &szFilename = QLatin1String(""));

    QString getClientInfoMessage();

    ///properties
    const QString           szWindowTitle = QStringLiteral("Logger Client");
//    QString                 szLoggerPattern = QStringLiteral("[%d][FILE:%c{1}.java]: %p %m"); //[2018-09-11 00:00:27,251][FILE:ServiceCloseOpenShift.java]: DEBUG (606020) Attempt to start close shift ended
    bool                    bUsingCustomColumnWidth;
    LogMode                 eCurrentMode;
    QString                 szSavedLogFile;
    bool                    bOpenFileAfterSavingPending;
    bool                    bIsAtBottom;

    ///elements
    ChannelSocketClient     *myChannelSocketClient;

    NetworkConnectionWidget *myServerConnectionWidget{};

    QPushButton             *buttonOpenFile{};

    LoggerPatternWidget     *myLoggerPatternWidget{};

    QPushButton             *pushButtonResizeColumns{};
    QPushButton             *pushButtonClearTable{};
    PushButtonWithMenu      *pushButtonSaveToFile{};
    QVector<QAction *>      myActionsSaveToFile;

    QPushButton             *pushButtonOptions{};
    OptionsWidget           *myOptionsWidget;

    LoggerTableProxyModel   *myProxyModel{};
    LoggerTreeView          *myTableView{};

    SearchWidget            *mySearchWidget{};
    QPushButton             *pushButtonClearFilter{};

    KeywordHighlightWidget  *myKeywordHighlightWidget{};

protected slots:
    void buttonConnectToServerToggled(bool bButtonState);

    void buttonOpenFileClicked(bool bButtonState);
    void buttonOpenFileResult(const QString &szFilename);
    void fileParsingResult(const int nResult, const QString &szFilename);

    void pasteText();
    void clipboardParsingResult(const GlobalConstants::ErrorCode eParsingResult);

    void rowsAboutToBeInsertedInModel(const QModelIndex &parent, int first, int last);
    void rowsInsertedInModel(const QModelIndex &parent, int first, int last);

    void rowsRemovedFromModel(const QModelIndex &parent, int first, int last);

    void buttonClickedClearTable(bool bState);

    void buttonClickedClearFilter(bool bState);

    void buttonClickedSaveToFile(QAction *myAction);
    void buttonSaveToFileResult(const QString &szFilename);

    void customContextMenuRequestedOnTableView(const QPoint &pos);

    void filterStateChanged(bool bState);

    void searchTextChanged(const QString &szText);

    void keywordHighlightChanged(const QStringList &szaKeywords);

private slots:
    void connectionSuccess(const QString &szError);
    void connectionError(int nSocketError, const QString &szError);
    void connectionInProgress();
//    void newMessageReceived(const QString &szMessage);

    void loggerPatternChanged(const QString &szLoggerPattern);

    void tableViewHeaderResized(int logicalIndex, int oldSize, int newSize);

    void resizeColumnsLoosely();

    void fontSizeChanged(const int nValue);
    void rowHeightBiasChanged(int nValue = INT_MAX);

signals:
    void clearModel();

    void loggerPatternChangedSignal(QString);
    void parseFile(QString);
    void parseClipboard();

//reimplemented
protected:
    void dragEnterEvent(QDragEnterEvent *myDragEvent) override;
    void dropEvent(QDropEvent *myDropEvent) override;

    void resizeEvent(QResizeEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

#ifdef DEBUG_STUFF
private:
    void initDebugFocusChanged();
#endif

};

