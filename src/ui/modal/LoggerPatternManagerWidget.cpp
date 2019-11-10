#include "LoggerPatternManagerWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "ui/ToastNotificationWidget.h"
#include "util/NetworkUtils.h"
#include "view/StandardItemView.h"

LoggerPatternManagerWidget::LoggerPatternManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();

    loadSettings();
}

LoggerPatternManagerWidget::~LoggerPatternManagerWidget() = default;

QStringList LoggerPatternManagerWidget::getRow(const QString &szFind, const LoggerPatternsEnum::Columns eColumn)
{
    QList<QStandardItem *> myMatches = tableModelPatterns->findItems(szFind, Qt::MatchExactly, eColumn);

    if (myMatches.isEmpty() == true) {
        return QStringList();
    }

    int nRow = myMatches.at(0)->row();

    QStringList szaMatches;
    szaMatches.reserve(LoggerPatternsEnum::COUNT_TABLE_COLUMNS);

    for (int nCol = 0; nCol < LoggerPatternsEnum::COUNT_TABLE_COLUMNS; ++nCol) {
        szaMatches.append(tableModelPatterns->item(nRow, nCol)->text());
    }

    return szaMatches;
}

int LoggerPatternManagerWidget::getRowIndex(const QString &szFind, const LoggerPatternsEnum::Columns eColumn)
{
    QList<QStandardItem *> myMatches = tableModelPatterns->findItems(szFind, Qt::MatchExactly, eColumn);

    if (myMatches.isEmpty() == true) {
        return -1;
    }

    return myMatches.at(0)->row();
}

QStringList LoggerPatternManagerWidget::getMatches(const QString &szFind, const LoggerPatternsEnum::Columns eColumn, const Qt::MatchFlags eMatchFlag)
{
    QList<QStandardItem *> myMatches = tableModelPatterns->findItems(szFind, eMatchFlag, eColumn);

    if (myMatches.isEmpty() == true) {
        return QStringList();
    }

    QStringList szaMatches;
    szaMatches.reserve(myMatches.size());

    for (const QStandardItem *myMatch : myMatches) {
        szaMatches.append(myMatch->text());
    }

    return szaMatches;
}

void LoggerPatternManagerWidget::setValues(const QString &szPatternName, const QString &szPattern)
{
    lineEditPatternName->setText(szPatternName);
    lineEditPattern->setText(szPattern);
}

void LoggerPatternManagerWidget::pushButtonSavePatternPushed(bool checked)
{
    Q_UNUSED(checked)

    const QString szPatternName = lineEditPatternName->text();
    const QString szPattern  = lineEditPattern->text();

    if (szPatternName.isEmpty() == true
        || szPattern.isEmpty() == true) {

        ToastNotificationWidget::showMessage(this, tr("Please fill all fields"), ToastNotificationWidget::ERROR, 2000);
        return;
    }

    int nRow = tableModelPatterns->indexOf(szPatternName, LoggerPatternsEnum::COLUMN_PATTERN_NAME);

    //found entry with same Name
    if (nRow != -1) {
        QMessageBox::StandardButton eReplaceItem = QMessageBox::question(this, tr("Replace pattern"), tr("Pattern name already exists, replace pattern?"), QMessageBox::Yes | QMessageBox::No);

        if (eReplaceItem == QMessageBox::Yes) {
            tableModelPatterns->item(nRow, LoggerPatternsEnum::COLUMN_PATTERN)->setText(szPatternName);

            tableModelPatterns->sort();
        }

        return;
    }

    nRow = tableModelPatterns->indexOf(szPattern, LoggerPatternsEnum::COLUMN_PATTERN);

    //found entry with same Pattern
    if (nRow != -1) {
        QMessageBox::StandardButton eReplaceItem = QMessageBox::question(this, tr("Replace pattern"), tr("Pattern already exists, replace name?"), QMessageBox::Yes | QMessageBox::No);

        if (eReplaceItem == QMessageBox::Yes) {
            tableModelPatterns->item(nRow, LoggerPatternsEnum::COLUMN_PATTERN_NAME)->setText(szPatternName);

            tableModelPatterns->sort();
        }

        return;
    }

    QList<QStandardItem *> myTableRow;

    myTableRow.append(new QStandardItem(szPatternName));
    myTableRow.append(new QStandardItem(szPattern));

    tableModelPatterns->appendRow(myTableRow);
}

void LoggerPatternManagerWidget::setupUi()
{
    int nCurrentRow = 0;

    QGridLayout *myMainLayout = new QGridLayout(this);
    myMainLayout->setContentsMargins(10, 10, 10, 10);
    myMainLayout->setSpacing(20);

    QHBoxLayout *myEditsLayout = new QHBoxLayout();
    {
        lineEditPatternName = new QLineEdit(this);
        lineEditPatternName->setPlaceholderText(tr("Pattern Name"));
        lineEditPatternName->setToolTip(tr("Pattern Name"));

        lineEditPattern = new QLineEdit(this);
        lineEditPattern->setPlaceholderText(tr("Pattern"));
        lineEditPattern->setToolTip(tr("Pattern"));

        pushButtonSavePattern = new QPushButton(this);
        pushButtonSavePattern->setCheckable(false);
        pushButtonSavePattern->setText(tr("Save"));

        connect(lineEditPatternName, SIGNAL(returnPressed()), pushButtonSavePattern, SLOT(click()));
        connect(lineEditPattern, SIGNAL(returnPressed()), pushButtonSavePattern, SLOT(click()));


        myEditsLayout->addWidget(lineEditPatternName);
        myEditsLayout->addWidget(lineEditPattern);
        myEditsLayout->addWidget(pushButtonSavePattern);
    }

    {
        tableModelPatterns = new LoggerPatternsModel(this);
        tableModelPatterns->setAllowDuplicates(false);
        tableModelPatterns->setSortColumn(LoggerPatternsEnum::COLUMN_PATTERN_NAME);

        tableViewPatterns = new StandardItemView(this);
        tableViewPatterns->setAlternatingRowColors(true);
        tableViewPatterns->setModel(tableModelPatterns);
        tableViewPatterns->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        tableViewPatterns->setSortingEnabled(true);
        tableViewPatterns->setShowGrid(true);
        tableViewPatterns->setContextMenuPolicy(Qt::CustomContextMenu);
        tableViewPatterns->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tableViewPatterns->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableViewPatterns->setWordWrap(false);

        //Horizontal
        tableViewPatterns->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableViewPatterns->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableViewPatterns->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        tableViewPatterns->horizontalHeader()->setHighlightSections(false);
        tableViewPatterns->horizontalHeader()->setStretchLastSection(true);
        //Vertical
        tableViewPatterns->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableViewPatterns->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        tableViewPatterns->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        tableViewPatterns->verticalHeader()->setVisible(false);
    }

    myMainLayout->addLayout(myEditsLayout, nCurrentRow, 0);
    ++nCurrentRow;
//    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
    myMainLayout->addWidget(tableViewPatterns, nCurrentRow, 0, 2, -1);

    this->setLayout(myMainLayout);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window  | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    this->resize(800, 600); //TODO doesn't work with large font sizes

    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));
    this->setWindowTitle(tr("Logger Pattern Manager"));

    this->hide();
}

void LoggerPatternManagerWidget::setupSignalsAndSlots()
{
    connect(this,                                   &LoggerPatternManagerWidget::patternChangeRequested,
            this,                                   [ = ] { this->hide(); });

    connect(pushButtonSavePattern,                  &QPushButton::clicked,
            this,                                   &LoggerPatternManagerWidget::pushButtonSavePatternPushed);

    connect(tableViewPatterns,                      &StandardItemView::activated,
            this,
    [ = ] (const QModelIndex & index) {
        if (index.isValid() == true) {
            emit patternChangeRequested(tableModelPatterns->item(index.row(), LoggerPatternsEnum::COLUMN_PATTERN_NAME)->text());
        }
    });
}

void LoggerPatternManagerWidget::loadSettings()
{
    const QString szLoggerPatterns = AppSettings::getValue(AppSettings::KEY_LOGGER_PATTERN_LIST, AppSettings::getDefaultValue(AppSettings::KEY_LOGGER_PATTERN_LIST)).toString();

    if (szLoggerPatterns.isEmpty() == false) {

        const QStringList szaLoggerPatterns = szLoggerPatterns.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

        for (int nRow = 0; nRow < szaLoggerPatterns.size(); ++nRow) {

            const QStringList szaLoggerPattern = szaLoggerPatterns.at(nRow).split(GlobalConstants::SEPARATOR_SETTINGS_LIST_2);

            if (szaLoggerPattern.size() != LoggerPatternsEnum::COUNT_TABLE_COLUMNS) {
                ToastNotificationWidget::showMessage(this, tr("Could not parse address: ") + szaLoggerPatterns.at(nRow), ToastNotificationWidget::ERROR, 3000);
                continue;
            }

            QList<QStandardItem *> myTableRow;

            for (int nCol = 0; nCol < LoggerPatternsEnum::COUNT_TABLE_COLUMNS; ++nCol) {
                myTableRow.append(new QStandardItem(szaLoggerPattern.at(nCol)));
            }

            tableModelPatterns->appendRow(myTableRow);
        }
    }
}

void LoggerPatternManagerWidget::saveSettings()
{
    QStringList szaLoggerPatterns;
    szaLoggerPatterns.reserve(tableModelPatterns->rowCount());

    for (int nRow = 0; nRow < tableModelPatterns->rowCount(); ++nRow) {
        QStringList szaLoggerPattern;

        for (int nColumn = 0; nColumn < LoggerPatternsEnum::COUNT_TABLE_COLUMNS; ++nColumn) {
            QString szText = tableModelPatterns->item(nRow, nColumn)->text();

            if (szText.trimmed().isEmpty() == true) {
                break;
            }

            szaLoggerPattern.append(szText);
        }

        if (szaLoggerPattern.size() != LoggerPatternsEnum::COUNT_TABLE_COLUMNS) {
            continue;
        }

        szaLoggerPatterns.append(szaLoggerPattern.join(GlobalConstants::SEPARATOR_SETTINGS_LIST_2));
    }

    AppSettings::setValue(AppSettings::KEY_LOGGER_PATTERN_LIST, szaLoggerPatterns.join(GlobalConstants::SEPARATOR_SETTINGS_LIST));
}

void LoggerPatternManagerWidget::hideEvent(QHideEvent *event)
{
    emit aboutToHide();

    QWidget::hideEvent(event);

    saveSettings();

    lineEditPatternName->clear();
    lineEditPattern->clear();
}

void LoggerPatternManagerWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToHide();

    QWidget::closeEvent(event);
}

