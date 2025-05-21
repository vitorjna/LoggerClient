#include "LoggerPatternWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "ui/ToastNotificationWidget.h"

LoggerPatternWidget::LoggerPatternWidget(QWidget *parent)
    : QWidget(parent)
    , myLoggerPatternManagerWidget(new LoggerPatternManagerWidget(this))
{
    setupUi();
    setupSignalsAndSlots();

    loadCurrentPattern();
}

LoggerPatternWidget::~LoggerPatternWidget() = default;

void LoggerPatternWidget::setPatternName(const QString &szPatternName)
{
    comboBoxLoggerPatternName->setCurrentText(szPatternName);
}

void LoggerPatternWidget::setPattern(const QString &szPattern)
{
    comboBoxLoggerPattern->setCurrentText(szPattern);
}

void LoggerPatternWidget::loadCurrentPattern()
{
    const QString szCurrentPattern = AppSettings::getValue(AppSettings::KEY_LOGGER_PATTERN, AppSettings::getDefaultValue(AppSettings::KEY_LOGGER_PATTERN)).toString();

    QStringList szaCurrentPattern = szCurrentPattern.split(GlobalConstants::SEPARATOR_SETTINGS_LIST);

    if (szaCurrentPattern.size() != 2) {
        ToastNotificationWidget::showMessage(nullptr, tr("Could not parse pattern: ") + szCurrentPattern + tr(". Using default"), ToastNotificationWidget::ERROR, 3000);

        szaCurrentPattern = AppSettings::getDefaultValue(AppSettings::KEY_LOGGER_PATTERN).toString().split(GlobalConstants::SEPARATOR_SETTINGS_LIST);
    }

    setPattern(szaCurrentPattern.at(1));
}

void LoggerPatternWidget::saveCurrentPattern()
{
    const QString szCurrentPattern = comboBoxLoggerPatternName->currentText() + GlobalConstants::SEPARATOR_SETTINGS_LIST + comboBoxLoggerPattern->currentText();

    AppSettings::setValue(AppSettings::KEY_LOGGER_PATTERN, szCurrentPattern);
}

void LoggerPatternWidget::setupUi()
{
    QHBoxLayout *myServerConnectionLayout = new QHBoxLayout(this);
    myServerConnectionLayout->setContentsMargins(0, 0, 0, 0);
    {
        labelLoggerPattern = new QLabel(this);
        labelLoggerPattern->setText(tr("Pattern:"));

        comboBoxLoggerPatternName = new QComboBox(this);
        comboBoxLoggerPatternName->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        comboBoxLoggerPatternName->setEditable(true);
        comboBoxLoggerPatternName->setModel(myLoggerPatternManagerWidget->getModel());
        comboBoxLoggerPatternName->setModelColumn(LoggerPatternsEnum::COLUMN_PATTERN_NAME);

        comboBoxLoggerPattern = new QComboBox(this);
        comboBoxLoggerPattern->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        comboBoxLoggerPattern->setEditable(true);
        comboBoxLoggerPattern->setModel(myLoggerPatternManagerWidget->getModel());
        comboBoxLoggerPattern->setModelColumn(LoggerPatternsEnum::COLUMN_PATTERN);

        buttonManagePatterns = new QPushButton(this);
        buttonManagePatterns->setCheckable(true);
        buttonManagePatterns->setText(tr("Manage"));
        buttonManagePatterns->setToolTip(tr("Manage logger patterns"));

        myServerConnectionLayout->addWidget(labelLoggerPattern);
        myServerConnectionLayout->addWidget(comboBoxLoggerPatternName);
        myServerConnectionLayout->addWidget(comboBoxLoggerPattern);
        myServerConnectionLayout->addSpacing(10);
        myServerConnectionLayout->addWidget(buttonManagePatterns);
    }

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

//    setMaximumWidth(INT16_MAX);
//    setMinimumWidth(500);
}

void LoggerPatternWidget::setupSignalsAndSlots()
{
    connect(comboBoxLoggerPatternName,          &QComboBox::editTextChanged,
            this,                               &LoggerPatternWidget::loggerPatternNameChanged);

    connect(comboBoxLoggerPattern,              &QComboBox::editTextChanged,
            this,                               &LoggerPatternWidget::loggerPatternChanged);

    connect(buttonManagePatterns,               &QPushButton::toggled,
            this,                               &LoggerPatternWidget::buttonManagePatternsToggled);

    connect(myLoggerPatternManagerWidget,       &LoggerPatternManagerWidget::aboutToHide,
            this,                               [ this ] { buttonManagePatterns->setChecked(false); } );

    connect(myLoggerPatternManagerWidget,       &LoggerPatternManagerWidget::patternChangeRequested,
            this,                               &LoggerPatternWidget::patternChangeRequested);

}

void LoggerPatternWidget::loggerPatternNameChanged(const QString &szPatternName)
{
    const int nRow = myLoggerPatternManagerWidget->getRowIndex(szPatternName, LoggerPatternsEnum::COLUMN_PATTERN_NAME);

    if (nRow == -1) {
        return;
    }

    comboBoxLoggerPattern->setCurrentIndex(nRow);
}

void LoggerPatternWidget::loggerPatternChanged(const QString &szPattern)
{
    Q_EMIT loggerPatternChangedSignal(szPattern);

    const int nRow = myLoggerPatternManagerWidget->getRowIndex(szPattern, LoggerPatternsEnum::COLUMN_PATTERN);

    comboBoxLoggerPatternName->setCurrentIndex(nRow); //if no match, name will be empty

    saveCurrentPattern();
}

void LoggerPatternWidget::buttonManagePatternsToggled(bool bButtonState)
{
    if (bButtonState == true) {
        myLoggerPatternManagerWidget->setValues(comboBoxLoggerPatternName->currentText(), comboBoxLoggerPattern->currentText());

        myLoggerPatternManagerWidget->show();

    } else {
        myLoggerPatternManagerWidget->hide();
        buttonManagePatterns->clearFocus();
    }
}

void LoggerPatternWidget::patternChangeRequested(const QString &szPattern)
{
    loggerPatternNameChanged(szPattern);
}

void LoggerPatternWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    comboBoxLoggerPatternName->setFocus();
}

