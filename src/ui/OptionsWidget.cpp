#include "OptionsWidget.h"
#include "application/AppSettings.h"
#include "application/GlobalConstants.h"
#include "application/SourceCodeHandler.h"
#include "util/FileUtils.h"

OptionsWidget::OptionsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupSignalsAndSlots();
    loadSettings();
}

OptionsWidget::~OptionsWidget()
{
}

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
        labelThemeChoice    = new QLabel(tr("Theme"), this);
        comboBoxThemeChoice = new QComboBox(this);
        comboBoxThemeChoice->addItems(loadThemeChoices());

        labelFontSize   = new QLabel(tr("Font size"), this);
        spinBoxFontSize = new QSpinBox(this);
        spinBoxFontSize->setRange(5, 18);

        labelRowHeightBias   = new QLabel(tr("Row height bias"), this);
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
        labelFormatExportedLogs = new QLabel(tr("Format exported logs"), this);

        checkBoxFormatExportedLogs = new QCheckBox(this);
    }
    myMainLayout->addWidget(labelFormatExportedLogs,    nCurrentRow, 0);
    myMainLayout->addWidget(checkBoxFormatExportedLogs, nCurrentRow, 1);
    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), nCurrentRow, 2);
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
            pushButtonSourceCodeLocationAdd = new QPushButton(this);
            pushButtonSourceCodeLocationAdd->setText(tr("Add location"));

            pushButtonSourceCodeLocationRemove = new QPushButton(this);
            pushButtonSourceCodeLocationRemove->setText(tr("Remove"));
            pushButtonSourceCodeLocationRemove->setEnabled(false);

            listWidgetSourceCodeLocations = new QListWidget(this);
            listWidgetSourceCodeLocations->addItems(SourceCodeHandler::getSourceCodeLocations());
            listWidgetSourceCodeLocations->setAlternatingRowColors(true);
            listWidgetSourceCodeLocations->setDragDropMode(QAbstractItemView::InternalMove);
            listWidgetSourceCodeLocations->setSelectionMode(QAbstractItemView::ExtendedSelection);

            sourceCodeLocationsLayout->addWidget(pushButtonSourceCodeLocationAdd,       0, 0);
            sourceCodeLocationsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
            sourceCodeLocationsLayout->addWidget(pushButtonSourceCodeLocationRemove,    0, 2);
            sourceCodeLocationsLayout->addWidget(listWidgetSourceCodeLocations,         1, 0, 2, -1);
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
    this->resize(600, 600);

    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));

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

    connect(pushButtonSourceCodeLocationAdd,        &QPushButton::clicked,
            this,                                   &OptionsWidget::buttonAddLocationClicked);

    connect(pushButtonSourceCodeLocationRemove,     &QPushButton::clicked,
            this,                                   &OptionsWidget::buttonRemoveLocationClicked);

    connect(listWidgetSourceCodeLocations,          &QListWidget::currentRowChanged,
            this,                                   &OptionsWidget::sourceLocationsListItemChanged);

    connect(buttonBoxCloseWindow,                   &QDialogButtonBox::rejected,
            this,                                   &OptionsWidget::close);
}

void OptionsWidget::loadSettings()
{
    QString szThemeSelected = AppSettings::getValue(AppSettings::KEY_THEME_NAME, GlobalConstants::SETTINGS_THEME_DEFAULT).toString();
    comboBoxThemeChoice->setCurrentText(szThemeSelected);

    int nFontSize = AppSettings::getValue(AppSettings::KEY_FONT_SIZE, 8).toInt();
    spinBoxFontSize->setValue(nFontSize);

    int nRowBias = AppSettings::getValue(AppSettings::KEY_ROW_HEIGHT_BIAS, 0).toInt();
    spinBoxRowHeightBias->setValue(nRowBias);

    bool bFormatExportedLogs = AppSettings::getValue(AppSettings::KEY_FORMAT_EXPORTED_LOGS, false).toBool();
    checkBoxFormatExportedLogs->setChecked(bFormatExportedLogs);

    QString szCodeLanguage = SourceCodeHandler::getCurrentLanguage();
    comboBoxSupportedLanguages->setCurrentText(szCodeLanguage);

    QString szEditorName = SourceCodeHandler::getCurrentEditorName();
    comboBoxCodeEditorNames->setCurrentText(szEditorName);

    lineEditEditorLocation->setText(SourceCodeHandler::getEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
    lineEditEditorHandling->setText(SourceCodeHandler::getEditorHandling(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
}

QStringList OptionsWidget::loadThemeChoices()
{
    QDir myThemesDir(GlobalConstants::SETTINGS_FOLDER_THEMES);

    QStringList szaThemesAvailable = myThemesDir.entryList(QStringList("*" + GlobalConstants::FILE_EXTENSION_QSS), QDir::Files, QDir::Name | QDir::IgnoreCase);

    szaThemesAvailable.replaceInStrings(GlobalConstants::FILE_EXTENSION_QSS, QLatin1String(""));

    return szaThemesAvailable;
}

void OptionsWidget::saveSourceCodeLocations()
{
    QStringList szaSourceFolders;

    for (int nIndex = 0; nIndex < listWidgetSourceCodeLocations->count(); ++nIndex) {
        szaSourceFolders << listWidgetSourceCodeLocations->item(nIndex)->text();
    }

    SourceCodeHandler::setSourceCodeLocations(szaSourceFolders);
}

void OptionsWidget::themeSelectionChanged(const QString &szNewTheme)
{
    QString szThemeFilename(GlobalConstants::SETTINGS_FOLDER_THEMES
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

void OptionsWidget::formatExportedLogsChanged(const int nState)
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

void OptionsWidget::codeEditorLanguageChanged(const QString &szNewLanguage)
{
    SourceCodeHandler::setCurrentLanguage(szNewLanguage);
}

void OptionsWidget::codeEditorSelectionChanged(const QString &szNewEditor)
{
    SourceCodeHandler::setCurrentEditor(szNewEditor);

    lineEditEditorLocation->setText(SourceCodeHandler::getEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
    lineEditEditorHandling->setText(SourceCodeHandler::getEditorHandling(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex())));
}

void OptionsWidget::codeEditorLocationChanged(const QString &szNewLocation)
{
    bool bFileExists = QFile::exists(szNewLocation);

    if (bFileExists == true) {
        lineEditEditorLocation->setStyleSheet(QLatin1String(""));
        SourceCodeHandler::setEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex()), szNewLocation);

    } else {
        lineEditEditorLocation->setStyleSheet(QStringLiteral("border: 1px solid red"));
    }
}

void OptionsWidget::buttonEditorLocationPickClicked(bool bState)
{
    Q_UNUSED(bState)

    QFileDialog *myFileDialog = new QFileDialog(this);
    myFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    myFileDialog->setFileMode(QFileDialog::ExistingFile);
    myFileDialog->setFilter(QDir::Files | QDir::Executable);

    myFileDialog->open(this, SLOT(buttonEditorLocationPickResult(QString)));
}

void OptionsWidget::buttonEditorLocationPickResult(const QString &szFile)
{
    lineEditEditorLocation->setText(szFile);
}

void OptionsWidget::buttonAddLocationClicked(bool bState)
{
    Q_UNUSED(bState)

    QFileDialog *myFileDialog = new QFileDialog(this);
    myFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    myFileDialog->setFileMode(QFileDialog::Directory);
    myFileDialog->setOption(QFileDialog::ShowDirsOnly, true);

    myFileDialog->open(this, SLOT(buttonAddLocationResult(QString)));
}

void OptionsWidget::buttonAddLocationResult(const QString &szFolder)
{
    listWidgetSourceCodeLocations->addItem(szFolder);

    saveSourceCodeLocations();
}

void OptionsWidget::buttonRemoveLocationClicked(bool bState)
{
    Q_UNUSED(bState)

    QList<QListWidgetItem *> mySelectedItems = listWidgetSourceCodeLocations->selectedItems();

    for (QListWidgetItem *mySelectedItem : mySelectedItems) {
        delete mySelectedItem;
    }

    if (listWidgetSourceCodeLocations->count() == 0) {
        pushButtonSourceCodeLocationRemove->setEnabled(false);
    }

    saveSourceCodeLocations();
}

void OptionsWidget::sourceLocationsListItemChanged(int nCurrentItem)
{
    Q_UNUSED(nCurrentItem)

    pushButtonSourceCodeLocationRemove->setEnabled(nCurrentItem != -1);
}

void OptionsWidget::hideEvent(QHideEvent *event)
{
    saveSourceCodeLocations(); //when source order is changed, the ListWidget doesn't emit a proper signal. So save the list when hiding/closing the window

    QWidget::hideEvent(event);
}

void OptionsWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToHide();

    QWidget::closeEvent(event);
}

