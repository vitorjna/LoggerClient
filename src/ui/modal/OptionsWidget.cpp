#include "OptionsWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "application/SourceCodeHandler.h"

OptionsWidget::OptionsWidget(QWidget *parent)
    : QWidget(parent)
    , mySourceCodeLocationWidget(new SourceCodeLocationWidget(this))
{
    setupUi();
    setupSignalsAndSlots();
    loadSettings();
}

void OptionsWidget::fontSizeChange(int nAmount)
{
    if (nAmount == 0) {
        spinBoxFontSize->setValue(AppSettings::getDefaultValue(AppSettings::KEY_FONT_SIZE).toInt());

    } else {
        spinBoxFontSize->stepBy(nAmount);
    }
}

void OptionsWidget::rowHeightBiasChange(int nAmount)
{
    if (nAmount == 0) {
        spinBoxRowHeightBias->setValue(AppSettings::getDefaultValue(AppSettings::KEY_ROW_HEIGHT_BIAS).toInt());

    } else {
        spinBoxRowHeightBias->stepBy(nAmount);
    }
}

OptionsWidget::~OptionsWidget() = default;

void OptionsWidget::setupUi()
{
    int nCurrentRow = 0;

    QGridLayout *myMainLayout = new QGridLayout(this);
    myMainLayout->setContentsMargins(10, 10, 10, 10);
    myMainLayout->setSpacing(20);

    QGroupBox *uiSettingsGroup = new QGroupBox(tr("UI Settings"));
    uiSettingsGroup->setFlat(true);
//    sourceCodeGroup->setStyleSheet("border-left:0;border-bottom:0;border-right:0;");

    QGridLayout *uiSettingsLayout = new QGridLayout(uiSettingsGroup);
    {
        QLabel *labelThemeChoice = new QLabel(tr("Theme"), this);
        comboBoxThemeChoice = new QComboBox(this);
        comboBoxThemeChoice->addItems(loadThemeChoices());

        QLabel *labelFontSize = new QLabel(tr("Font size"), this);
        spinBoxFontSize = new QSpinBox(this);
        spinBoxFontSize->setRange(4, 24);

        QLabel *labelRowHeightBias = new QLabel(tr("Row height bias"), this);
        spinBoxRowHeightBias = new QSpinBox(this);
        spinBoxRowHeightBias->setRange(-5, 5);

        uiSettingsLayout->addWidget(labelThemeChoice,       0, 0);
        uiSettingsLayout->addWidget(comboBoxThemeChoice,    0, 1);
        uiSettingsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 2);

        uiSettingsLayout->addWidget(labelFontSize,          1, 0);
        uiSettingsLayout->addWidget(spinBoxFontSize,        1, 1);
        uiSettingsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 1, 2);
        uiSettingsLayout->addWidget(labelRowHeightBias,     1, 3);
        uiSettingsLayout->addWidget(spinBoxRowHeightBias,   1, 4);
    }
    myMainLayout->addWidget(uiSettingsGroup, nCurrentRow, 0, 1, -1);
    ++nCurrentRow;


    {
        QLabel *labelFormatExportedLogs = new QLabel(tr("Format exported logs"), this);

        checkBoxFormatExportedLogs = new QCheckBox(this);

        myMainLayout->addWidget(labelFormatExportedLogs,    nCurrentRow, 0);
        myMainLayout->addWidget(checkBoxFormatExportedLogs, nCurrentRow, 1);
        myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), nCurrentRow, 2);
    }
    ++nCurrentRow;


    QGroupBox *sourceCodeGroup = new QGroupBox(tr("Log Classes and Source Code"));
    sourceCodeGroup->setFlat(true);
//    sourceCodeGroup->setStyleSheet("border-left:0;border-bottom:0;border-right:0;");

    QGridLayout *sourceCodeLayout = new QGridLayout(sourceCodeGroup);
    {
        QLabel *sourceCodeTypeLabel = new QLabel(this);
        sourceCodeTypeLabel->setText(tr("Code language"));

        comboBoxSupportedLanguages = new QComboBox(this);
        comboBoxSupportedLanguages->setMinimumContentsLength(5);
        comboBoxSupportedLanguages->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
        comboBoxSupportedLanguages->addItems(SourceCodeHandler::getSupportedLanguages());

        sourceCodeLayout->addWidget(sourceCodeTypeLabel,        0,  0);
        sourceCodeLayout->addWidget(comboBoxSupportedLanguages, 0,  1);
        sourceCodeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 2);

        //-------------------------------------------------
        QGroupBox *sourceCodeEditorGroup = new QGroupBox(tr("Editor to open"));
        QGridLayout *sourceCodeEditorLayout = new QGridLayout(sourceCodeEditorGroup);
        {
            comboBoxCodeEditorNames = new QComboBox(this);
            comboBoxCodeEditorNames->addItems(SourceCodeHandler::getEditorNames());

            lineEditEditorHandling = new QLineEdit(this);
            lineEditEditorHandling->setPlaceholderText(tr("Arguments to open application"));
            lineEditEditorHandling->setEnabled(false);

            lineEditEditorLocation = new QLineEdit(this);
            lineEditEditorLocation->setPlaceholderText(tr("Editor location"));
            lineEditEditorLocation->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

            pushButtonEditorLocationPick = new QPushButton(this);
            pushButtonEditorLocationPick->setText(tr("Choose"));

            sourceCodeEditorLayout->addWidget(comboBoxCodeEditorNames,      0, 0, 1, 2);
            sourceCodeEditorLayout->addWidget(lineEditEditorHandling,       0, 2, 1, -1);
            sourceCodeEditorLayout->addWidget(lineEditEditorLocation,       1, 0, 1, 4);
            sourceCodeEditorLayout->addWidget(pushButtonEditorLocationPick, 1, 4, 1, -1);
        }
        sourceCodeLayout->addWidget(sourceCodeEditorGroup, 1, 0, 1, -1);

        //-------------------------------------------------
        QGroupBox *sourceCodeLocationsGroup = new QGroupBox(tr("Source code locations"));
        QGridLayout *sourceCodeLocationsLayout = new QGridLayout(sourceCodeLocationsGroup);
        {
            QLabel *labelActiveProject = new QLabel(tr("Project"), this);
            comboBoxActiveProjectName = new QComboBox(this);
            comboBoxActiveProjectName->addItems(mySourceCodeLocationWidget->getProjectNames());
            comboBoxActiveProjectName->model()->sort(0);

            sourceCodeLocationsLayout->addWidget(labelActiveProject,            0, 0);
            sourceCodeLocationsLayout->addWidget(comboBoxActiveProjectName,     0, 1);
            sourceCodeLocationsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 2);
            sourceCodeLocationsLayout->addWidget(mySourceCodeLocationWidget,    1, 0, -1, -1);
        }
        sourceCodeLayout->addWidget(sourceCodeLocationsGroup,  2,  0, 1, -1);

    }
    myMainLayout->addWidget(sourceCodeGroup, nCurrentRow, 0, 1, -1);
    ++nCurrentRow;

    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), nCurrentRow, 0);
    ++nCurrentRow;

    buttonBoxCloseWindow = new QDialogButtonBox(this);
    buttonBoxCloseWindow->setStandardButtons(QDialogButtonBox::Close);

    myMainLayout->addWidget(buttonBoxCloseWindow, nCurrentRow, 0, 1, -1, Qt::AlignRight);


    this->setLayout(myMainLayout);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window  | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    this->resize(800, 800); //TODO doesn't work with large font sizes

    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));
    this->setWindowTitle(tr("Options"));

    this->hide();
}

void OptionsWidget::setupSignalsAndSlots()
{
    connect(comboBoxThemeChoice,                    &QComboBox::currentTextChanged,
            this,                                   &OptionsWidget::themeSelectionChanged);

    connect(spinBoxFontSize,                        static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), //there is an overload and the lambda connection cannot disambiguate
            this,                                   &OptionsWidget::fontSizeChangedSlot);

    connect(spinBoxRowHeightBias,                   static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), //there is an overload and the lambda connection cannot disambiguate
            this,                                   &OptionsWidget::rowHeightBiasChangedSlot);

    connect(checkBoxFormatExportedLogs,             &QCheckBox::stateChanged,
            this,                                   &OptionsWidget::formatExportedLogsChanged);

    connect(comboBoxSupportedLanguages,             &QComboBox::currentTextChanged,
            this,                                   &OptionsWidget::codeEditorLanguageChanged);

    connect(comboBoxCodeEditorNames,                &QComboBox::currentTextChanged,
            this,                                   &OptionsWidget::codeEditorSelectionChanged);

    connect(lineEditEditorLocation,                 &QLineEdit::textChanged,
            this,                                   &OptionsWidget::codeEditorLocationChanged);

    connect(pushButtonEditorLocationPick,           &QPushButton::clicked,
            this,                                   &OptionsWidget::buttonEditorLocationPickClicked);

    connect(comboBoxActiveProjectName,              &QComboBox::currentTextChanged,
            this,                                   &OptionsWidget::activeProjectNameSelectionChanged);

    connect(mySourceCodeLocationWidget,             &SourceCodeLocationWidget::newProjectAdded,
    this,                                           [ this ] (const QString & szProjectName) {
        comboBoxActiveProjectName->addItem(szProjectName);
        comboBoxActiveProjectName->model()->sort(0);
    }
           );

    connect(mySourceCodeLocationWidget,             &SourceCodeLocationWidget::projectListChanged,
    this,                                           [ this ] () {
        comboBoxActiveProjectName->clear();
        comboBoxActiveProjectName->addItems(mySourceCodeLocationWidget->getProjectNames());
        comboBoxActiveProjectName->model()->sort(0);
    }
           );

    connect(buttonBoxCloseWindow,                   &QDialogButtonBox::rejected,
            this,                                   &OptionsWidget::close);
}

void OptionsWidget::loadSettings()
{
    const QString szThemeSelected = AppSettings::getValue(AppSettings::KEY_THEME_NAME, AppSettings::getDefaultValue(AppSettings::KEY_THEME_NAME)).toString();
    comboBoxThemeChoice->setCurrentText(szThemeSelected);

    const int nFontSize = AppSettings::getValue(AppSettings::KEY_FONT_SIZE, AppSettings::getDefaultValue(AppSettings::KEY_FONT_SIZE)).toInt();
    spinBoxFontSize->setValue(nFontSize);

    const int nRowBias = AppSettings::getValue(AppSettings::KEY_ROW_HEIGHT_BIAS, AppSettings::getDefaultValue(AppSettings::KEY_ROW_HEIGHT_BIAS)).toInt();
    spinBoxRowHeightBias->setValue(nRowBias);

    const bool bFormatExportedLogs = AppSettings::getValue(AppSettings::KEY_FORMAT_EXPORTED_LOGS, false).toBool();
    checkBoxFormatExportedLogs->setChecked(bFormatExportedLogs);

    const QString szCodeLanguage = SourceCodeHandler::getCurrentLanguage();
    comboBoxSupportedLanguages->setCurrentText(szCodeLanguage);

    const QString szEditorName = SourceCodeHandler::getCurrentEditorName();
    comboBoxCodeEditorNames->setCurrentText(szEditorName);

    const QString szCurrentProject = SourceCodeHandler::getCurrentProjectName();
    comboBoxActiveProjectName->setCurrentText(szCurrentProject);

    lineEditEditorLocation->setText(SourceCodeHandler::getEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
    lineEditEditorHandling->setText(SourceCodeHandler::getEditorArguments(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())).join(' '));
}

QStringList OptionsWidget::loadThemeChoices() const
{
    const QDir myThemesDir(GlobalConstants::SETTINGS_FOLDER_THEMES);

    QStringList szaThemesAvailable = myThemesDir.entryList(QStringList("*" + GlobalConstants::FILE_EXTENSION_QSS), QDir::Files, QDir::Name | QDir::IgnoreCase);

    szaThemesAvailable.replaceInStrings(GlobalConstants::FILE_EXTENSION_QSS, QStringLiteral(""));

    return szaThemesAvailable;
}

void OptionsWidget::themeSelectionChanged(const QString &szNewTheme) const
{
    const QString szThemeFilename(GlobalConstants::SETTINGS_FOLDER_THEMES
                                  + "/"
                                  + szNewTheme
                                  + GlobalConstants::FILE_EXTENSION_QSS);

    if (QFileInfo::exists(szThemeFilename) == false) {
        return;
    }

    QFile fileStyleSheet(szThemeFilename);

    if (fileStyleSheet.open(QIODevice::ReadOnly | QIODevice::Text | QIODevice::Unbuffered) == true) {
        QTextStream streamStyleSheet(&fileStyleSheet);

        qApp->setStyleSheet(streamStyleSheet.readAll());

        fileStyleSheet.close();

    } else {
        qDebug() << "Could not load stylesheet " << szNewTheme;
    }

    AppSettings::setValue(AppSettings::KEY_THEME_NAME, szNewTheme);
}

void OptionsWidget::fontSizeChangedSlot(const int nValue)
{
    AppSettings::setValue(AppSettings::KEY_FONT_SIZE, nValue);

    QFont myAppFont = qApp->font();
    myAppFont.setPointSize(nValue);

    qApp->setFont(myAppFont);
    qApp->setStyleSheet(qApp->styleSheet()); //force a complete refresh

    emit fontSizeChanged(nValue);
}

void OptionsWidget::rowHeightBiasChangedSlot(const int nValue)
{
    emit rowHeightBiasChanged(nValue);

    AppSettings::setValue(AppSettings::KEY_ROW_HEIGHT_BIAS, nValue);
}

void OptionsWidget::formatExportedLogsChanged(const int nState) const
{
    switch (static_cast<Qt::CheckState>(nState)) {
        case Qt::Unchecked:
        case Qt::PartiallyChecked:
            AppSettings::setValue(AppSettings::KEY_FORMAT_EXPORTED_LOGS, false);
            AppSettings::setValue(AppSettings::KEY_FORMAT_EXPORTED_LOGS, false);
            break;

        case Qt::Checked:
            AppSettings::setValue(AppSettings::KEY_FORMAT_EXPORTED_LOGS, true);
            break;
    }
}

void OptionsWidget::codeEditorLanguageChanged(const QString &szNewLanguage) const
{
    SourceCodeHandler::setCurrentLanguage(szNewLanguage);
}

void OptionsWidget::codeEditorSelectionChanged(const QString &szNewEditor)
{
    SourceCodeHandler::setCurrentEditor(szNewEditor);

    lineEditEditorLocation->setText(SourceCodeHandler::getEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
    lineEditEditorHandling->setText(SourceCodeHandler::getEditorArguments(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())).join(' '));
}

void OptionsWidget::codeEditorLocationChanged(const QString &szNewLocation)
{
    const bool bFileExists = QFile::exists(szNewLocation);

    if (bFileExists == true) {
        lineEditEditorLocation->setStyleSheet(QStringLiteral(""));
        SourceCodeHandler::setEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex()), szNewLocation);

    } else {
        lineEditEditorLocation->setStyleSheet(QStringLiteral("border: 1px solid red"));
    }
}

void OptionsWidget::activeProjectNameSelectionChanged(const QString &szProjectName) const
{
    SourceCodeHandler::setCurrentProjectName(szProjectName);
}

void OptionsWidget::buttonEditorLocationPickClicked(bool bState)
{
    Q_UNUSED(bState)

    QFileDialog *myFileDialog = new QFileDialog(this);
    myFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    myFileDialog->setFileMode(QFileDialog::ExistingFile);
    myFileDialog->setFilter(QDir::Files | QDir::Executable);

    myFileDialog->open(this, SLOT(buttonEditorLocationPickResult(QString)));

    QObject::connect(myFileDialog, &QFileDialog::fileSelected,
    this, [ myFileDialog, this ](const QString & szFolder) {
        lineEditEditorLocation->setText(szFolder);

        myFileDialog->deleteLater();
    }
                    );
}

void OptionsWidget::hideEvent(QHideEvent *event)
{
    emit aboutToHide();

    QWidget::hideEvent(event);
}

void OptionsWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToHide();

    QWidget::closeEvent(event);
}

