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
    {
        labelThemeChoice = new QLabel(tr("Theme"), this);

        comboBoxThemeChoice = new QComboBox(this);
        comboBoxThemeChoice->addItems(loadThemeChoices());

//        mySearchTextLayout->addWidget(pushButtonNextResult);
    }
    myMainLayout->addWidget(labelThemeChoice,       nCurrentRow, 0);
    myMainLayout->addWidget(comboBoxThemeChoice,    nCurrentRow, 1);
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
        sourceCodeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 1);
        sourceCodeLayout->addWidget(comboBoxSupportedLanguages, 0,  2, 1, 2);

        //-------------------------------------------------
        QGroupBox *sourceCodeEditorGroup = new QGroupBox(tr("Editor to open"));
        QGridLayout *sourceCodeEditorLayout = new QGridLayout(sourceCodeEditorGroup);
        {
            QLabel *sourceCodeEditorLabel = new QLabel(this);
            sourceCodeEditorLabel->setText(tr("Application"));

            comboBoxCodeEditorNames = new QComboBox(this);
            comboBoxCodeEditorNames->addItems(SourceCodeHandler::getEditorNames());

            sourceCodeEditorLayout->addWidget(sourceCodeEditorLabel,    0,  0);
            sourceCodeEditorLayout->addWidget(comboBoxCodeEditorNames,  0,  1);

            lineEditEditorLocation = new QLineEdit(this);
            lineEditEditorLocation->setPlaceholderText(tr("Editor location"));

            lineEditEditorHandling = new QLineEdit(this);
            lineEditEditorHandling->setPlaceholderText(tr("Arguments to open application"));
            lineEditEditorHandling->setEnabled(false);

            sourceCodeEditorLayout->addWidget(lineEditEditorLocation,   1,  0, 1, -1);
            sourceCodeEditorLayout->addWidget(lineEditEditorHandling,   2,  0, 1, -1);
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

            sourceCodeLocationsLayout->addWidget(pushButtonSourceCodeLocationAdd,       0, 0);
            sourceCodeLocationsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);
            sourceCodeLocationsLayout->addWidget(pushButtonSourceCodeLocationRemove,    0, 2);
            sourceCodeLocationsLayout->addWidget(listWidgetSourceCodeLocations,         1, 0, 2, -1);
        }
        sourceCodeLayout->addWidget(sourceCodeLocationsGroup,  2,  0, 1, -1);

    }
    myMainLayout->addWidget(sourceCodeGroup,        nCurrentRow, 0, 1, -1);
    ++nCurrentRow;

    myMainLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding), nCurrentRow, 0);
    ++nCurrentRow;

    buttonBoxCloseWindow = new QDialogButtonBox(this);
    buttonBoxCloseWindow->setStandardButtons(QDialogButtonBox::Close);

    myMainLayout->addWidget(buttonBoxCloseWindow,   nCurrentRow, 0, 1, -1, Qt::AlignRight);


    this->setLayout(myMainLayout);
    this->setAttribute(Qt::WA_DeleteOnClose, false);
    this->setWindowFlags(Qt::Window  | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    this->resize(600, 600);

    this->setWindowIcon(QIcon(":/icons/themes/icons/appIcon.svg"));

    this->hide();
}

void OptionsWidget::setupSignalsAndSlots()
{
    connect(comboBoxThemeChoice,        &QComboBox::currentTextChanged,
            this,                       &OptionsWidget::themeSelectionChanged);

    connect(buttonBoxCloseWindow,       &QDialogButtonBox::rejected,
            this,                       &OptionsWidget::close);

    connect(comboBoxSupportedLanguages, &QComboBox::currentTextChanged,
            this,                       &OptionsWidget::codeEditorLanguageChanged);

    connect(comboBoxCodeEditorNames,    &QComboBox::currentTextChanged,
            this,                       &OptionsWidget::codeEditorSelectionChanged);

    connect(lineEditEditorLocation,     &QLineEdit::textEdited,
            this,                       &OptionsWidget::codeEditorLocationChanged);

    connect(pushButtonSourceCodeLocationAdd,        &QPushButton::clicked,
            this,                                   &OptionsWidget::buttonAddLocationClicked);

    connect(pushButtonSourceCodeLocationRemove,     &QPushButton::clicked,
            this,                                   &OptionsWidget::buttonRemoveLocationClicked);

    connect(listWidgetSourceCodeLocations,          &QListWidget::currentRowChanged,
            this,                                   &OptionsWidget::sourceLocationsListItemChanged);
}

void OptionsWidget::loadSettings()
{
    QString szThemeSelected = AppSettings::getValue(AppSettings::KEY_THEME_NAME, GlobalConstants::SETTINGS_THEME_DEFAULT).toString();
    comboBoxThemeChoice->setCurrentText(szThemeSelected);

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
    SourceCodeHandler::setEditorLocation(static_cast<SourceCodeHandler::SourceCodeEditors>(comboBoxCodeEditorNames->currentIndex()), szNewLocation);
}

void OptionsWidget::buttonAddLocationClicked(bool bState)
{
    Q_UNUSED(bState)

    QFileDialog *myFileDialog = new QFileDialog(this);
    myFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
    myFileDialog->setFileMode(QFileDialog::Directory);

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

    delete listWidgetSourceCodeLocations->takeItem(listWidgetSourceCodeLocations->currentRow());

    saveSourceCodeLocations();
}

void OptionsWidget::sourceLocationsListItemChanged(int nCurrentItem)
{
    Q_UNUSED(nCurrentItem)

    pushButtonSourceCodeLocationRemove->setEnabled(true);
}

void OptionsWidget::closeEvent(QCloseEvent *event)
{
    emit aboutToHide();

    QWidget::closeEvent(event);
}

